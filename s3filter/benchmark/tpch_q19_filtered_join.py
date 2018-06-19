# -*- coding: utf-8 -*-
"""TPCH Q19 Filtered Benchmark

"""

import os

from s3filter import ROOT_DIR
from s3filter.benchmark.tpch_q19_ops import filter_def
from s3filter.op.aggregate import Aggregate
from s3filter.op.aggregate_expression import AggregateExpression
from s3filter.op.collate import Collate
from s3filter.op.hash_join import HashJoin
from s3filter.op.nested_loop_join import JoinExpression
from s3filter.op.project import Project, ProjectExpression
from s3filter.op.sql_table_scan import SQLTableScan
from s3filter.plan.query_plan import QueryPlan
from s3filter.util.test_util import gen_test_id


def collate_op():
    return Collate('collate', False)


def aggregate_def():
    return Aggregate(
        [
            AggregateExpression(
                AggregateExpression.SUM,
                lambda t_: float(t_['l_extendedprice']) * float((1 - float(t_['l_discount']))))
        ],
        'aggregate',
        False)


def join_op():
    return HashJoin(JoinExpression('l_partkey', 'p_partkey'), 'lineitem_part_join', False)


def aggregate_project_def():
    return Project(
        [
            ProjectExpression(lambda t_: t_['_0'], 'revenue')
        ],
        'aggregate_project',
        False)


def main():
    """

    :return: None
    """

    print('')
    print("TPCH Q19 Filtered Join")
    print("----------------------")

    query_plan = QueryPlan()

    # Define the operators

    # with lineitem_scan as (select * from lineitem)
    lineitem_scan = query_plan.add_operator(
        SQLTableScan('lineitem.csv',
                     "select "
                     "  l_partkey, "
                     "  l_quantity, "
                     "  l_extendedprice, "
                     "  l_discount, "
                     "  l_shipinstruct, "
                     "  l_shipmode "
                     "from "
                     "  S3Object "
                     "where "
                     "  ( "
                     "      ( "
                     "          cast(l_quantity as integer) >= 3 and cast(l_quantity as integer) <= 3 + 10 "
                     "          and l_shipmode in ('AIR', 'AIR REG') "
                     "          and l_shipinstruct = 'DELIVER IN PERSON' "
                     "      ) "
                     "      or "
                     "      ( "
                     "          cast(l_quantity as integer) >= 16 and cast(l_quantity as integer) <= 16 + 10 "
                     "          and l_shipmode in ('AIR', 'AIR REG') "
                     "          and l_shipinstruct = 'DELIVER IN PERSON' "
                     "      ) "
                     "      or "
                     "      ( "
                     "          cast(l_quantity as integer) >= 24 and cast(l_quantity as integer) <= 24 + 10 "
                     "          and l_shipmode in ('AIR', 'AIR REG') "
                     "          and l_shipinstruct = 'DELIVER IN PERSON' "
                     "      ) "
                     "  ) ",
                     'lineitem_scan',
                     False))

    # with part_scan as (select * from part)
    part_scan = query_plan.add_operator(
        SQLTableScan('part.csv',
                     "select "
                     "  p_partkey, "
                     "  p_brand, "
                     "  p_size, "
                     "  p_container "
                     "from "
                     "  S3Object "
                     "where "
                     "  ( "
                     "      ( "
                     "          p_brand = 'Brand#11' "
                     "          and p_container in ("
                     "              'SM CASE', "
                     "              'SM BOX', "
                     "              'SM PACK', "
                     "              'SM PKG'"
                     "          ) "
                     "          and cast(p_size as integer) between 1 and 5 "
                     "      ) "
                     "      or "
                     "      ( "
                     "          p_brand = 'Brand#44' "
                     "          and p_container in ("
                     "              'MED BAG', "
                     "              'MED BOX', "
                     "              'MED PKG', "
                     "              'MED PACK'"
                     "          ) "
                     "          and cast(p_size as integer) between 1 and 10 "
                     "      ) "
                     "      or "
                     "      ( "
                     "          p_brand = 'Brand#53' "
                     "          and p_container in ("
                     "              'LG CASE', "
                     "              'LG BOX', "
                     "              'LG PACK', "
                     "              'LG PKG'"
                     "          ) "
                     "          and cast(p_size as integer) between 1 and 15 "
                     "      ) "
                     "  ) ",
                     'part_scan',
                     False))

    # with lineitem_project as (
    # select
    # _1 as l_partkey, _4 as l_quantity,_5 as l_quantity, 6 as l_quantity, 13 as l_quantity, 14 as l_quantity
    # from lineitem_scan
    # )
    lineitem_project = query_plan.add_operator(Project(
        [
            ProjectExpression(lambda t_: t_['_0'], 'l_partkey'),
            ProjectExpression(lambda t_: t_['_1'], 'l_quantity'),
            ProjectExpression(lambda t_: t_['_2'], 'l_extendedprice'),
            ProjectExpression(lambda t_: t_['_3'], 'l_discount'),
            ProjectExpression(lambda t_: t_['_4'], 'l_shipinstruct'),
            ProjectExpression(lambda t_: t_['_5'], 'l_shipmode')
        ],
        'lineitem_project',
        False))

    # with part_project as (
    # select
    # _0 as p_partkey, _3 as p_brand, _5 as p_size, _6 as p_container
    # from part_scan
    # )
    part_project = query_plan.add_operator(Project(
        [
            ProjectExpression(lambda t_: t_['_0'], 'p_partkey'),
            ProjectExpression(lambda t_: t_['_1'], 'p_brand'),
            ProjectExpression(lambda t_: t_['_2'], 'p_size'),
            ProjectExpression(lambda t_: t_['_3'], 'p_container')
        ],
        'part_project',
        False))

    lineitem_part_join = query_plan.add_operator(join_op())
    filter_op = query_plan.add_operator(filter_def())
    aggregate = query_plan.add_operator(aggregate_def())
    aggregate_project = query_plan.add_operator(aggregate_project_def())
    collate = query_plan.add_operator(collate_op())

    # Connect the operators
    lineitem_scan.connect(lineitem_project)
    part_scan.connect(part_project)

    lineitem_part_join.connect_left_producer(lineitem_project)
    lineitem_part_join.connect_right_producer(part_project)

    lineitem_part_join.connect(filter_op)
    filter_op.connect(aggregate)
    aggregate.connect(aggregate_project)
    aggregate_project.connect(collate)

    # Write the plan graph
    query_plan.write_graph(os.path.join(ROOT_DIR, "../tests-output"), gen_test_id())

    # Start the query
    part_scan.start()
    lineitem_scan.start()

    # Assert the results
    num_rows = 0
    for t in collate.tuples():
        num_rows += 1
        # print("{}:{}".format(num_rows, t))

    collate.print_tuples()

    # Write the metrics
    query_plan.print_metrics()

    field_names = ['revenue']

    assert len(collate.tuples()) == 1 + 1

    assert collate.tuples()[0] == field_names

    # NOTE: This result has been verified with the equivalent data and query on PostgreSQL
    assert collate.tuples()[1] == [3468861.097000001]
