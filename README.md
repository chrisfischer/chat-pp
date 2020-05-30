# Ĉ++ (Chat++)

Ĉ++ is a distributed chat application with that supports multiple backend servers communicating internally over multicast and bidirectional streams. All networking is handled with gRPC. Supports distributed voting for chat room entrance and kick. See a video demo [here](https://drive.google.com/open?id=11L2qauT6zQQAOrsguDlhcI-UPd6yJxjg).

To build, run `bazel build src:all`.