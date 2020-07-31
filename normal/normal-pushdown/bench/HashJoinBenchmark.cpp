//
// Created by matt on 31/7/20.
//

#include <memory>

#include <doctest/doctest.h>

#include <nanobench.h>

#include <normal/tuple/Sample.h>
#include <normal/pushdown/join/HashJoinBuildKernel.h>
#include <normal/pushdown/join/HashJoinProbeKernel.h>

using namespace normal::tuple;
using namespace normal::pushdown;
using namespace normal::pushdown::join;

#define SKIP_SUITE false

void run(const std::shared_ptr<TupleSet2> &tupleSet1, const std::shared_ptr<TupleSet2> &tupleSet2) {

  TupleSet2 joinedTupleSet;

  ankerl::nanobench::Config().minEpochIterations(1).run(
	  fmt::format("join-{}-{}-rows", tupleSet1->numRows(), tupleSet2->numRows()), [&] {
		auto buildKernel = HashJoinBuildKernel::make("c_0");
		buildKernel.put(tupleSet1);
		auto hashTable = buildKernel.getHashTable();

		auto probeKernel = HashJoinProbeKernel::make(JoinPredicate("c_0", "c_0"));
		probeKernel.putHashTable(hashTable);
		probeKernel.putTupleSet(tupleSet2);
		joinedTupleSet = *probeKernel.join().value();
	  });

  SPDLOG_DEBUG("Output:\n{}", joinedTupleSet.showString(TupleSetShowOptions(TupleSetShowOrientation::RowOriented)));

}

TEST_SUITE ("join-benchmark" * doctest::skip(SKIP_SUITE)) {

TEST_CASE ("join-benchmark-scaling" * doctest::skip(false || SKIP_SUITE)) {

  auto tupleSet10 = Sample::sampleCxRIntString(10, 10);
  auto tupleSet100 = Sample::sampleCxRIntString(10, 100);
  auto tupleSet1000 = Sample::sampleCxRIntString(10, 1000);
//  auto tupleSet10000 = Sample::sampleCxRIntString(10, 10000);
//  auto tupleSet100000 = Sample::sampleCxRIntString(10, 100000);

  run(tupleSet10, tupleSet100);
  run(tupleSet100, tupleSet1000);
//  run(tupleSet1000, tupleSet10000);
//  run(tupleSet10000, tupleSet100000);
}

}