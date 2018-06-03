# -*- coding: utf-8 -*-
"""Projection support

"""
from plan.op_metrics import OpMetrics
from op.operator_base import Operator
from op.message import TupleMessage
from op.tuple import Tuple, LabelledTuple


class ProjectExpression(object):
    """Represents a projection expression, where one record field can be renamed to another.

    """

    def __init__(self, expr, new_field_name):
        """Creates a new projection expression

        :param expr: The projection expression
        :param new_field_name The new field name.
        """

        self.expr = expr
        self.new_field_name = new_field_name.strip()


class Project(Operator):
    """This operator mapes fields from the received tuples using the given projection expressions and maps the given
    alias to each of them. It does not block but rather emits tuples as they are received.

    """

    def __init__(self, project_exprs, name, log_enabled):
        """Constructs a new Project operator.

        :param project_exprs: The expressions defining the projection.
        :param name: The operator name
        :param log_enabled: Whether logging is enabled
        """

        super(Project, self).__init__(name, OpMetrics(), log_enabled)

        self.project_exprs = project_exprs

        self.field_names = None

    def on_receive(self, m, _producer):
        """Handles the event of receiving a new message from a producer.

        :param m: The received tuple
        :param _producer: The producer of the tuple
        :return: None
        """

        # print("Project | {}".format(t))

        if type(m) is TupleMessage:
            self.on_receive_tuple(m.tuple_)
        else:
            raise Exception("Unrecognized message {}".format(m))

    def on_receive_tuple(self, tuple_):
        """Handles the receipt of a tuple. The tuple is mapped to a new tuple using the given projection expressions.
        The field names are modified according to the new field names in the projection expressions.

        :param tuple_: The received tuple
        :return: None
        """

        if self.log_enabled:
            print("{}('{}') | {}".format(self.__class__.__name__, self.name, tuple_))

        if not self.field_names:

            self.field_names = tuple_

            # Map the old field names to the new
            projected_field_names = []
            for e in self.project_exprs:
                fn = e.new_field_name
                projected_field_names.append(fn)

            self.send(TupleMessage(Tuple(projected_field_names)), self.consumers)

        else:

            # Perform the projection using the given expressions
            lt = LabelledTuple(tuple_, self.field_names)

            projected_field_values = []
            for e in self.project_exprs:
                fv = e.expr(lt)
                projected_field_values.append(fv)

            self.send(TupleMessage(Tuple(projected_field_values)), self.consumers)
