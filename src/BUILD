load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

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
    linkopts = ["-lpthread"],
    deps = [
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
        ":forwarding_service_client",
        ":forwarding_service_impl",
        ":server_state",
    ],
)

cc_library(
    name = "client_server_api",
    srcs = ["client_server_api.cpp"],
    hdrs = ["client_server_api.hpp"],
    linkopts = ["-lpthread"],
    deps = [
        "//proto:client_server_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "client",
    srcs = ["client.cpp"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "//proto:client_server_cc_grpc",
	":client_server_api",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
