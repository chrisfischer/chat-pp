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
  ]
)

cc_library(
  name = "server_server_api",
  srcs = ["server_server_api.cpp"],
  hdrs = ["server_server_api.hpp"],
  deps = [
    ":server_state",
    "//proto:client_server_cc_grpc",
    "//proto:server_server_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
  ],
)

cc_binary(
  name = "server",
  srcs = ["server.cpp"],
  defines = ["BAZEL_BUILD"],
  deps = [
    ":server_state",
    ":server_server_api",
  ],
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