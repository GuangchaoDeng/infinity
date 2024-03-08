//  Copyright(C) 2023 InfiniFlow, Inc. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

module;

#include <map> // fix nlohmann::json compile error in release mode
module fast_rough_filter;
import stl;
import value;
import default_values;
import probabilistic_data_filter;
import min_max_data_filter;
import logger;
import third_party;
import local_file_system;
import infinity_exception;
import filter_expression_push_down_helper;

namespace infinity {

void FastRoughFilter::SetHaveStartedBuildTask(TxnTimeStamp begin_ts) {
    std::lock_guard lock(mutex_check_task_start_);
    if (build_time_ != UNCOMMIT_TS) {
        UnrecoverableError("FastRoughFilter::SetHaveStartedBuildTask(): Job already started.");
    }
    build_time_ = begin_ts;
}

bool FastRoughFilter::HaveStartedBuildTask() const {
    std::lock_guard lock(mutex_check_task_start_);
    return build_time_ != UNCOMMIT_TS;
}

String FastRoughFilter::SerializeToString() const {
    if (HaveFilter()) {
        u32 probabilistic_data_filter_binary_bytes = probabilistic_data_filter_->GetSerializeSizeInBytes();
        u32 min_max_data_filter_binary_bytes = min_max_data_filter_->GetSerializeSizeInBytes();
        u32 total_binary_bytes =
            sizeof(total_binary_bytes) + sizeof(build_time_) + probabilistic_data_filter_binary_bytes + min_max_data_filter_binary_bytes;
        String save_to_binary;
        save_to_binary.reserve(total_binary_bytes);
        OStringStream os(std::move(save_to_binary));
        os.write(reinterpret_cast<const char *>(&total_binary_bytes), sizeof(total_binary_bytes));
        os.write(reinterpret_cast<const char *>(&build_time_), sizeof(build_time_));
        probabilistic_data_filter_->SerializeToStringStream(os, probabilistic_data_filter_binary_bytes);
        min_max_data_filter_->SerializeToStringStream(os, min_max_data_filter_binary_bytes);
        if (os.view().size() != total_binary_bytes) {
            UnrecoverableError("BUG: FastRoughFilter::SerializeToString(): save size error");
        }
        return std::move(os).str();
    } else {
        UnrecoverableError("FastRoughFilter::SerializeToString(): No FastRoughFilter data.");
        return {};
    }
}

void FastRoughFilter::DeserializeFromString(const String &str) {
    if (HaveFilter()) [[unlikely]] {
        UnrecoverableError("BUG: FastRoughFilter::DeserializeFromString(): Already have data.");
    }
    // load
    IStringStream is(str);
    u32 total_binary_bytes;
    is.read(reinterpret_cast<char *>(&total_binary_bytes), sizeof(total_binary_bytes));
    if (total_binary_bytes != is.view().size()) {
        UnrecoverableError("FastRoughFilter::DeserializeFromString(): load size error");
    }
    is.read(reinterpret_cast<char *>(&build_time_), sizeof(build_time_));
    probabilistic_data_filter_ = MakeUnique<ProbabilisticDataFilter>();
    probabilistic_data_filter_->DeserializeFromStringStream(is);
    min_max_data_filter_ = MakeUnique<MinMaxDataFilter>();
    min_max_data_filter_->DeserializeFromStringStream(is);
    // check position
    if (!is or u32(is.tellg()) != is.view().size()) {
        UnrecoverableError("FastRoughFilter::DeserializeFromString(): load size error");
    }
    FinishBuildFastRoughFilterTask();
}

void FastRoughFilter::SaveToJsonFile(nlohmann::json &entry_json) const {
    if (HaveFilter()) {
        //  LOG_TRACE("FastRoughFilter::SaveToJsonFile(): have FastRoughFilter, save now.");
        entry_json[JsonTagBuildTime] = build_time_;
        probabilistic_data_filter_->SaveToJsonFile(entry_json);
        min_max_data_filter_->SaveToJsonFile(entry_json);
    } else {
        UnrecoverableError("FastRoughFilter::SaveToJsonFile(): No FastRoughFilter data.");
    }
}

// will throw in caller function if return false
// called in deserialize, remove unnecessary lock
bool FastRoughFilter::LoadFromJsonFile(const nlohmann::json &entry_json) {
    if (HaveFilter()) [[unlikely]] {
        UnrecoverableError("BUG: FastRoughFilter::LoadFromJsonFile(): Already have data.");
    }
    // LOG_TRACE("FastRoughFilter::LoadFromJsonFile(): try to load filter data from json.");
    // try load JsonTagBuildTime first
    if (!entry_json.contains(JsonTagBuildTime)) {
        LOG_TRACE("FastRoughFilter::LoadFromJsonFile(): found no save data in json, stop loading.");
        return false;
    }
    // can load from json
    bool load_success = true;
    build_time_ = entry_json[JsonTagBuildTime];
    {
        // load ProbabilisticDataFilter
        auto load_probabilistic_data_filter = MakeUnique<ProbabilisticDataFilter>();
        if (load_probabilistic_data_filter->LoadFromJsonFile(entry_json)) {
            probabilistic_data_filter_ = std::move(load_probabilistic_data_filter);
            // LOG_TRACE("FastRoughFilter::LoadFromJsonFile(): Finish load ProbabilisticDataFilter data from json.");
        } else {
            load_success = false;
            LOG_TRACE("FastRoughFilter::LoadFromJsonFile(): Cannot load ProbabilisticDataFilter data from json.");
        }
    }
    {
        // load MinMaxDataFilter
        auto load_min_max_data_filter = MakeUnique<MinMaxDataFilter>();
        if (load_min_max_data_filter->LoadFromJsonFile(entry_json)) {
            min_max_data_filter_ = std::move(load_min_max_data_filter);
            // LOG_TRACE("FastRoughFilter::LoadFromJsonFile(): Finish load MinMaxDataFilter data from json.");
        } else {
            load_success = false;
            LOG_TRACE("FastRoughFilter::LoadFromJsonFile(): Cannot load MinMaxDataFilter data from json.");
        }
    }
    if (load_success) {
        FinishBuildFastRoughFilterTask();
        // LOG_TRACE("FastRoughFilter::LoadFromJsonFile(): successfully load FastRoughFilter data from json.");
    } else {
        LOG_ERROR("FastRoughFilter::LoadFromJsonFile(): partially failed to load FastRoughFilter data from json.");
    }
    return load_success;
}

} // namespace infinity