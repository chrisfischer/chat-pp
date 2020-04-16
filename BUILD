load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
  name = "server",
  srcs = ["server.cpp"],
  defines = ["BAZEL_BUILD"],
  deps = [
    "//proto:server_server_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
  ],
)