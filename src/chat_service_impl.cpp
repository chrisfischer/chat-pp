#include "src/chat_service_impl.hpp"

grpc::Status SendMessage(grpc::ServerContext* context,
            const client_server::Message* request,
            client_server::MessageResult* response) {
    return grpc::Status::OK;
}