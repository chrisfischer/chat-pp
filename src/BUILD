load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "server_state",
  srcs = ["server_state.cpp"],
  hdrs = ["server_state.hpp"],
  deps = [
    "//proto:client_server_cc_grpc",
    "@boost//:uuid",
  ],
)

cc_library(
  name = "chat_service_impl",
  srcs = ["chat_service_impl.cpp"],
  hdrs = ["chat_service_impl.hpp"],
  deps = [
    ":forwarding_service_client",
    "//proto:client_server_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
  ],
)

cc_library(
  name = "forwarding_service_client",
  srcs = ["forwarding_service_client.cpp"],
  hdrs = ["forwarding_service_client.hpp"],
  deps = [
    "//proto:client_server_cc_grpc",
    "//proto:server_server_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
  ],
  linkopts = ["-lpthread"],
)

cc_library(
  name = "forwarding_service_impl",
  srcs = ["forwarding_service_impl.cpp"],
  hdrs = ["forwarding_service_impl.hpp"],
  deps = [
    ":server_state",
    ":chat_service_impl",
    "//proto:client_server_cc_grpc",
    "//proto:server_server_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
  ],
  linkopts = ["-lpthread"],
)

cc_binary(
  name = "server",
  srcs = ["server.cpp"],
  defines = ["BAZEL_BUILD"],
  deps = [
    ":server_state",
    ":forwarding_service_client",
    ":forwarding_service_impl",
    ":chat_service_impl",
  ],
  linkopts = ["-lpthread"],
)

cc_binary(
  name = "client",
  srcs = ["client.cpp"],
  defines = ["BAZEL_BUILD"],
  deps = [
    "//proto:client_server_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
  ],
)