# -*- coding: utf-8 -*-
"""

"""

from op.operator_base import Operator
from op.message import TupleMessage, BloomMessage
from op.sql_table_scan import SQLTableScanMetrics
from op.tuple import Tuple, LabelledTuple
from sql.cursor import Cursor


class SQLTableScanBloomUse(Operator):
    """Performs a table scan using a received bloom filter.

    """

    def __init__(self, s3key, s3sql, bloom_filter_field_name, name, log_enabled):
        """

        :param s3key: The s3 key to select against
        :param s3sql:  The s3 sql to use
        :param bloom_filter_field_name: The field name to apply to the bloom filter predicate
        :param name: The name of the operator
        :param log_enabled: Whether logging is enabled
        """

        super(SQLTableScanBloomUse, self).__init__(name, SQLTableScanMetrics(), log_enabled)

        self.s3key = s3key
        self.s3sql = s3sql

        self.__field_names = None
        self.__tuples = []

        self.__bloom_filter = None

        if type(bloom_filter_field_name) is str:
            self.__bloom_filter_field_name = bloom_filter_field_name
        else:
            raise Exception("Bloom filter field name is of type {}. Field name must be of type str to be "
                            "used in SQL predicate".format(type(bloom_filter_field_name)))

    def on_producer_completed(self, _producer):
        """This event is overridden because we don't want the normal operator completion procedure to run. We want this
        operator to complete when all the tuples have been retrieved or consumers indicate they need no more tuples.

        :param _producer: The completed producer
        :return: None
        """

        pass

    def on_receive(self, m, _producer):
        """Handles the event of receiving a new message from a producer.

        :param m: The received message
        :param _producer: The producer of the message
        :return: None
        """

        # print("BloomScan | {}".format(t))

        if type(m) is BloomMessage:
            self.__bloom_filter = m.bloom_filter
            self.execute()
        else:
            raise Exception("Unrecognized message {}".format(m))

    def execute(self):
        """Executes the query and sends the tuples to consumers.

        :return: None
        """

        self.op_metrics.time_to_first_response_timer.start()

        bloom_filter_sql_predicate = self.__bloom_filter.sql_predicate(self.__bloom_filter_field_name)

        # Append the bloom filter predicate either using where... or and...
        sql_suffix = self.build_sql_suffix(bloom_filter_sql_predicate)

        sql = self.s3sql + sql_suffix
        cur = Cursor().select(self.s3key, sql)

        tuples = cur.execute()

        first_tuple = True
        for t in tuples:

            if self.is_completed():
                break

            self.op_metrics.time_to_first_response_timer.stop()

            self.op_metrics.rows_scanned += 1

            if first_tuple:
                self.send_field_names(t)
                first_tuple = False

            self.send_field_values(t)

        if not self.is_completed():
            self.complete()

    def build_sql_suffix(self, bloom_filter_sql_predicate):
        """Creates the bloom filter sql predicate. Basically determines whether the sql suffix should start with 'and'
        or 'where'.

        :param bloom_filter_sql_predicate: The sql predicate from the bloom filter
        :return: The sql suffix
        """

        stripped_sql = self.s3sql.strip()
        if stripped_sql.endswith(';'):
            stripped_sql = stripped_sql[:-1].strip()

        split_sql = stripped_sql.split()
        is_predicate_present = split_sql[-1].lower() is not "s3object"

        if is_predicate_present:
            return " and {} ".format(bloom_filter_sql_predicate)
        else:
            return " where {} ".format(bloom_filter_sql_predicate)

    def send_field_values(self, tuple_):
        """Sends a field values tuple

        :param tuple_: The tuple
        :return: None
        """

        if self.log_enabled:
            print("{}('{}') | Sending field values [{}]".format(self.__class__.__name__, self.name, {'data': tuple_}))

        self.send(TupleMessage(Tuple(tuple_)), self.consumers)

    def send_field_names(self, tuple_):
        """Sends the field names tuple

        :param tuple_: The tuple
        :return: None
        """

        # Create and send the record field names
        lt = LabelledTuple(tuple_)
        labels = Tuple(lt.labels)

        if self.log_enabled:
            print("{}('{}') | Sending field names [{}]".format(
                self.__class__.__name__,
                self.name,
                {'field_names': labels}))

        self.send(TupleMessage(labels), self.consumers)

    def __repr__(self):
        return "{}({})".format(self.__class__.__name__, {'name': self.name, 's3key': self.s3key})
