add_library(process-collector proc_reader.cc)

target_link_libraries(process-collector PUBLIC node_data_collect_protocol_proto spdlog::spdlog libcore::event)
