load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

package(default_visibility = ["//visibility:public"])

proto_library(
    name = "server_server_proto",
    srcs = ["server_server.proto"],
    deps = [":client_server_proto"],
)

cc_proto_library(
    name = "server_server_cc_proto",
    deps = [":server_server_proto"],
)

cc_grpc_library(
    name = "server_server_cc_grpc",
    srcs = [":server_server_proto"],
    grpc_only = True,
    deps = [":server_server_cc_proto"],
)

proto_library(
    name = "client_server_proto",
    srcs = ["client_server.proto"],
)

cc_proto_library(
    name = "client_server_cc_proto",
    deps = [":client_server_proto"],
)

cc_grpc_library(
    name = "client_server_cc_grpc",
    srcs = [":client_server_proto"],
    grpc_only = True,
    deps = [":client_server_cc_proto"],
)
