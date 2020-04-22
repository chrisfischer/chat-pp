#ifndef FORWARDING_SERIVCE_IMPL_HPP_
#define FORWARDING_SERIVCE_IMPL_HPP_

#include "server_state.hpp"
#include "chat_service_impl.hpp"

#include <grpcpp/grpcpp.h>
#include "proto/client_server.grpc.pb.h"
#include "proto/server_server.grpc.pb.h"

class ForwardingServiceImpl final : public SS::ForwardingService::Service {
private:
    std::shared_ptr<ServerState> state;
    std::shared_ptr<ChatServiceImpl> chat_service;

public:
    ForwardingServiceImpl(std::shared_ptr<ServerState> state,
            std::shared_ptr<ChatServiceImpl> chat_service);
    grpc::Status Forward(grpc::ServerContext* context,
            const SS::MessageRequest* request,
            client_server::Empty* response) override;
};

#endif /* FORWARDING_SERIVCE_IMPL_HPP_ */