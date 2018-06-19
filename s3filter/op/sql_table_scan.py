# -*- coding: utf-8 -*-
"""

"""

from s3filter.op.message import TupleMessage
from s3filter.op.operator_base import Operator
from s3filter.op.tuple import Tuple, IndexedTuple
from s3filter.plan.op_metrics import OpMetrics
from s3filter.sql.cursor import Cursor


class SQLTableScanMetrics(OpMetrics):
    """Extra metrics for a sql table scan

    """

    def __init__(self):
        super(SQLTableScanMetrics, self).__init__()

        self.rows_returned = 0

        self.time_to_first_response = 0
        self.time_to_first_record_response = None
        self.time_to_last_record_response = None

        self.bytes_scanned = 0
        self.bytes_processed = 0
        self.bytes_returned = 0

    def __repr__(self):
        return {
            'elapsed_time': round(self.elapsed_time(), 5),
            'rows_returned': self.rows_returned,
            'bytes_scanned': self.bytes_scanned,
            'bytes_processed': self.bytes_processed,
            'bytes_returned': self.bytes_returned,
            'time_to_first_response': round(self.time_to_first_response, 5),
            'time_to_first_record_response':
                None if self.time_to_first_record_response is None
                else round(self.time_to_first_record_response, 5),
            'time_to_last_record_response':
                None if self.time_to_last_record_response is None
                else round(self.time_to_last_record_response, 5)

        }.__repr__()


class SQLTableScan(Operator):
    """Represents a table scan operator which reads from an s3 table and emits tuples to consuming operators
    as they are received. Generally starting this operator is what begins a query.

    """

    def __init__(self, s3key, s3sql, name, log_enabled):
        """Creates a new Table Scan operator using the given s3 object key and s3 select sql

        :param s3key: The object key to select against
        :param s3sql: The s3 select sql
        """

        super(SQLTableScan, self).__init__(name, SQLTableScanMetrics(), log_enabled)

        self.s3key = s3key
        self.s3sql = s3sql

    def start(self):
        """Executes the query and begins emitting tuples.

        :return: None
        """

        self.op_metrics.timer_start()

        cur = Cursor().select(self.s3key, self.s3sql)

        tuples = cur.execute()

        self.op_metrics.time_to_first_response = self.op_metrics.elapsed_time()

        first_tuple = True
        for t in tuples:

            if self.is_completed():
                break

            self.op_metrics.rows_returned += 1

            if first_tuple:
                # Create and send the record field names
                it = IndexedTuple.build_default(t)
                first_tuple = False

                if self.log_enabled:
                    print("{}('{}') | Sending field names: {}"
                          .format(self.__class__.__name__, self.name, it.field_names()))

                self.send(TupleMessage(Tuple(it.field_names())), self.consumers)

            if self.log_enabled:
                print("{}('{}') | Sending field values: {}".format(self.__class__.__name__, self.name, t))

            self.send(TupleMessage(Tuple(t)), self.consumers)

        if not self.is_completed():
            self.complete()

        self.op_metrics.bytes_scanned = cur.bytes_scanned
        self.op_metrics.bytes_processed = cur.bytes_processed
        self.op_metrics.bytes_returned = cur.bytes_returned

        self.op_metrics.time_to_first_record_response = cur.time_to_first_record_response
        self.op_metrics.time_to_last_record_response = cur.time_to_last_record_response

        self.op_metrics.timer_stop()

    def on_producer_completed(self, _producer):
        """This event is overridden really just to indicate that it never fires.

        :param _producer: The completed producer
        :return: None
        """

        pass
