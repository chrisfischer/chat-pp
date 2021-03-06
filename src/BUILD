load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    srcs = ["common.cpp"],
    hdrs = ["common.hpp"],
    linkopts = ["-lpthread"],
    deps = [],
)

cc_library(
    name = "server_state",
    srcs = ["server_state.cpp"],
    hdrs = ["server_state.hpp"],
    linkopts = ["-lpthread"],
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
        ":common",
        ":forwarding_service_client",
        ":server_state",
        "//proto:client_server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name = "forwarding_service_client",
    srcs = ["forwarding_service_client.cpp"],
    hdrs = ["forwarding_service_client.hpp"],
    linkopts = ["-lpthread"],
    deps = [
        ":common",
        ":server_state",
        "//proto:client_server_cc_grpc",
        "//proto:server_server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name = "forwarding_service_impl",
    srcs = ["forwarding_service_impl.cpp"],
    hdrs = ["forwarding_service_impl.hpp"],
    linkopts = ["-lpthread"],
    deps = [
        ":chat_service_impl",
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
    linkopts = ["-lpthread"],
    deps = [
        ":chat_service_impl",
        ":common",
        ":forwarding_service_client",
        ":forwarding_service_impl",
        ":server_state",
    ],
)

cc_library(
    name = "client_state",
    srcs = ["client_state.cpp"],
    hdrs = ["client_state.hpp"],
    linkopts = ["-lpthread"],
    deps = ["//proto:client_server_cc_grpc"],
)

cc_library(
    name = "chat_service_client",
    srcs = ["chat_service_client.cpp"],
    hdrs = ["chat_service_client.hpp"],
    linkopts = ["-lpthread"],
    deps = [
        ":client_state",
        ":common",
        "//proto:client_server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "client",
    srcs = ["client.cpp"],
    defines = ["BAZEL_BUILD"],
    linkopts = ["-lpthread"],
    deps = [
        ":chat_service_client",
        ":client_state",
        ":common",
        "//proto:client_server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
