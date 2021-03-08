//
// Created by Yifei Yang on 3/2/21.
//

#include "MathModelTest.h"
#include <normal/sql/Interpreter.h>
#include <normal/plan/mode/Modes.h>
#include <normal/plan/Globals.h>
#include <normal/cache/FBRSCachingPolicy.h>
#include "ExperimentUtil.h"
#include <normal/pushdown/filter/Filter.h>
#include <normal/pushdown/Globals.h>
#include <numeric>

using namespace normal::ssb;
using namespace normal::sql;
using namespace normal::pushdown;

// not the best solution, should make a header file down the road most likely but this will work for now
void configureS3ConnectorMultiPartition(normal::sql::Interpreter &i, std::string bucket_name, std::string dir_prefix);
std::shared_ptr<TupleSet2> executeSql(normal::sql::Interpreter &i, const std::string &sql, bool saveMetrics, bool writeResults = false, std::string outputFileName = "");

std::string showMeasurementMetrics(const Interpreter& i, const std::shared_ptr<normal::plan::operator_::mode::Mode>& mode) {
  // collect
  std::stringstream formattedProcessingTime1;
  formattedProcessingTime1 << i.getExecutionTimes()[0] << " secs";

  std::stringstream formattedProcessedBytes1;
  formattedProcessedBytes1 << i.getBytesTransferred()[0].first << " B" << " ("
                           << ((double)i.getBytesTransferred()[0].first / 1024.0 / 1024.0 / 1024.0) << " GB)";

  std::stringstream formattedReturnedBytes1;
  formattedReturnedBytes1 << i.getBytesTransferred()[0].second << " B" << " ("
                          << ((double)i.getBytesTransferred()[0].second / 1024.0 / 1024.0 / 1024.0) << " GB)";

  std::stringstream formattedGetTransferConvertRate;
  formattedGetTransferConvertRate.precision(4);
  if (i.getGetTransferConvertNs()[0].first > 0 && i.getGetTransferConvertNs()[0].second > 0) {
    formattedGetTransferConvertRate << ((double) i.getBytesTransferred()[0].second / 1024.0 / 1024.0) /
                                       (((double) i.getGetTransferConvertNs()[0].first + i.getGetTransferConvertNs()[0].second) / 1.0e9) << " MB/s/req";
  } else {
    formattedGetTransferConvertRate << "NA";
  }

  std::stringstream formattedSelectTransferConvertRate;
  formattedSelectTransferConvertRate.precision(4);
  if (i.getSelectTransferConvertNs()[0].first > 0 && i.getSelectTransferConvertNs()[0].second > 0) {
    formattedSelectTransferConvertRate << ((double) i.getBytesTransferred()[0].second / 1024.0 / 1024.0) /
                                          ((double) (i.getSelectTransferConvertNs()[0].first + i.getSelectTransferConvertNs()[0].second) / 1.0e9) << " MB/s/req";
  } else {
    formattedSelectTransferConvertRate << "NA";
  }

  std::stringstream formattedS3SelectSelectivity;
  if (i.getBytesTransferred()[0].second && i.getSelectTransferConvertNs()[0].first > 0 && i.getGetTransferConvertNs()[0].first == 0) {
    formattedS3SelectSelectivity << (double) i.getBytesTransferred()[0].second / (double) i.getBytesTransferred()[0].first;
  } else {
    formattedS3SelectSelectivity << "NA";
  }

  std::stringstream formattedHitRatio;
  formattedHitRatio << i.getHitRatios()[0];

  // format
  std::stringstream ss;
  ss << std::left << std::setw(20) << mode->toString();
  ss << std::left << std::setw(18) << formattedProcessingTime1.str();
  ss << std::left << std::setw(30) << formattedProcessedBytes1.str();
  ss << std::left << std::setw(30) << formattedReturnedBytes1.str();
  ss << std::left << std::setw(25) << formattedGetTransferConvertRate.str();
  ss << std::left << std::setw(25) << formattedSelectTransferConvertRate.str();
  ss << std::left << std::setw(22) << formattedS3SelectSelectivity.str();
  ss << std::left << std::setw(15) << formattedHitRatio.str();
  ss << std::endl;

  return ss.str();
}

double measureLocalSpeed(normal::sql::Interpreter& i, filesystem::path& sql_file_dir_path) {
  SPDLOG_INFO("Measurement for local bandwidth:");
  filter::recordSpeeds = true;

  i.clearMetrics();
  i.clearHitRatios();
  auto sql_file_path = sql_file_dir_path.append(fmt::format("{}.sql", 2));
  auto sql = ExperimentUtil::read_file(sql_file_path.string());
  executeSql(i, sql, true, false, "");
  sql_file_dir_path = sql_file_dir_path.parent_path();
  SPDLOG_INFO("Query 1 for local bandwidth finished, hit ratio: {}", i.getHitRatios()[0]);
  auto time1 = i.getExecutionTimes()[0];
  auto megaBytesFiltered = (double)filter::totalBytesFiltered_ / 1024.0 / 1024.0;
  filter::totalBytesFiltered_ = 0;

//  i.clearMetrics();
//  i.clearHitRatios();
//  sql_file_path = sql_file_dir_path.append(fmt::format("{}.sql", 3));
//  sql = ExperimentUtil::read_file(sql_file_path.string());
//  executeSql(i, sql, true, false, "");
//  sql_file_dir_path = sql_file_dir_path.parent_path();
//  SPDLOG_INFO("Query 2 for local bandwidth finished, hit ratio: {}", i.getHitRatios()[0]);
//  auto time2 = i.getExecutionTimes()[0];

  filter::recordSpeeds = false;
  return megaBytesFiltered / (time1);
}

void normal::ssb::mathModelTest(size_t networkLimit) {  // unit: B/s
  spdlog::set_level(spdlog::level::info);
  std::stringstream ss;
  ss << std::setfill(' ');
  ss << std::left << std::setw(180) << std::setfill('-') << "" << std::endl;
  ss << std::setfill(' ');
  ss << std::left << std::setw(20) << "Mode";
  ss << std::left << std::setw(18) << "Execution Time";
  ss << std::left << std::setw(30) << "Processed Bytes";
  ss << std::left << std::setw(30) << "Returned Bytes";
  ss << std::left << std::setw(25) << "GET Transfer+Convert";
  ss << std::left << std::setw(25) << "SELECT Transfer+Convert";
  ss << std::left << std::setw(22) << "% Data S3 Selected";
  ss << std::left << std::setw(15) << "Hit ratio";
  ss << std::endl;
  ss << std::left << std::setw(180) << std::setfill('-') << "" << std::endl;
  ss << std::setfill(' ');

  double localSpeed;
  if (networkLimit > 0) {
    normal::pushdown::NetworkLimit = networkLimit;
    normal::plan::DefaultS3Client = normal::pushdown::AWSClient::defaultS3Client();
  }

  // parameters
  const size_t cacheSize = 12L * 1024 * 1024 * 1024;
  std::string bucket_name = "pushdowndb";
  std::string dir_prefix = "ssb-sf100-sortlineorder/csv/";

  // modes
  std::vector<std::shared_ptr<normal::plan::operator_::mode::Mode>> modes;
  modes.emplace_back(normal::plan::operator_::mode::Modes::fullPushdownMode());
  modes.emplace_back(normal::plan::operator_::mode::Modes::pullupCachingMode());
  modes.emplace_back(normal::plan::operator_::mode::Modes::hybridCachingMode());

  // test on each mode
  for (auto const& mode: modes) {
    auto cachingPolicy = FBRSCachingPolicy::make(cacheSize, mode);  // caching policy doesn't matter here
    auto sql_file_dir_path = filesystem::current_path().append("sql/generated");
    normal::sql::Interpreter i(mode, cachingPolicy);
    configureS3ConnectorMultiPartition(i, bucket_name, dir_prefix);
    SPDLOG_INFO("{} mode:", mode->toString());
    i.boot();

    // query for caching
    if (mode->id() != normal::plan::operator_::mode::ModeId::FullPullup &&
        mode->id() != normal::plan::operator_::mode::ModeId::FullPushdown) {
      SPDLOG_INFO("Query for caching:");
      auto sql_file_path = sql_file_dir_path.append(fmt::format("{}.sql", 1));
      auto sql = ExperimentUtil::read_file(sql_file_path.string());
      executeSql(i, sql, true, false, "");
      sql_file_dir_path = sql_file_dir_path.parent_path();
    }

    normal::cache::allowFetchSegments = false;
    i.clearMetrics();
    i.clearHitRatios();

    // query for measurement
    SPDLOG_INFO("Query for measurement:");
    auto sql_file_path = sql_file_dir_path.append(fmt::format("{}.sql", 2));
    auto sql = ExperimentUtil::read_file(sql_file_path.string());
    executeSql(i, sql, true, false, "");
    sql_file_dir_path = sql_file_dir_path.parent_path();
    SPDLOG_INFO("{} mode finished\nExecution metrics:\n{}", mode->toString(), i.showMetrics());
    SPDLOG_INFO("Cache hit ratios:\n{}", i.showHitRatios());
    ss << showMeasurementMetrics(i, mode);

    normal::cache::allowFetchSegments = true;

    // query for measuring local bandwidth
    if (mode->id() == normal::plan::operator_::mode::ModeId::PullupCaching) {
      localSpeed = measureLocalSpeed(i, sql_file_dir_path);
    }

    i.getOperatorGraph().reset();
    i.stop();
  }

  std::stringstream formattedLocalSpeed;
  formattedLocalSpeed << localSpeed << " MB/s";

  std::stringstream formattedNetworkSpeed;
  if (networkLimit > 0)
    formattedNetworkSpeed << (double) (networkLimit / 1024.0 / 1024.0) << " MB/s";
  else
    formattedNetworkSpeed << "Unlimited";

  ss << std::left << std::setw(20) << "Local bandwidth:";
  ss << std::left << std::setw(20) << formattedLocalSpeed.str();
  ss << std::endl;
  ss << std::left << std::setw(20) << "Network bandwidth:";
  ss << std::left << std::setw(20) << formattedNetworkSpeed.str();
  ss << std::endl;

  SPDLOG_INFO("Metrics summary:\n{}", ss.str());

  // Output to file
  auto metricsFilePath = filesystem::current_path().append("math_model_metrics");
  std::ofstream fout;
  fout.open(metricsFilePath.string(), std::ofstream::out | std::ofstream::app);
  fout << ss.str();
  fout.flush();
  fout.close();
}