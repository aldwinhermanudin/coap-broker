#ifndef COAP_BROKER_HPP
#define COAP_BROKER_HPP

#include <limits.h> 
#include <string>
#include "LinkedListDB.hpp"
#include "coap.h"
#include "LibcoapMod.hpp"
#include "LinkFormatParser.hpp"
#include "UString.hpp"
#include "Resource.hpp"
#include "Server.hpp"
namespace coap{

namespace broker{
    namespace handler{
        void tae_handler(ProtocolDataUnit response_pdu);
        void mr_handler(ProtocolDataUnit response_pdu);
        void uscf_handler(ProtocolDataUnit response_pdu);
        bool has_only_digits(const std::string s);
        bool is_ct_valid(ProtocolDataUnit request, unsigned short type);

        namespace post{
            time_t abs_ma(OptionList);
            void s_handler(LinkFormat lf_data, ProtocolDataUnit response_pdu,time_t abs_topic_ma);
            bool is_mr(ProtocolDataUnit request, LinkFormat lf_data);
            bool is_uscf(LinkFormat lf_data);
            bool is_tae(LinkFormat lf_data);
        }
    }
    namespace sub{
        namespace handler{
            namespace put{
                int abs_ct_value(OptionList request_opt);
                time_t abs_ma_value(OptionList request_opt);
                bool is_br(OptionList);
                bool is_uscf(Resource parent_resource, OptionList request_opt);
                bool is_s();
            } 
            namespace get{
                bool is_observe_notify_response(Resource current_resource,Address peer_addr, UString token_data, ProtocolDataUnit request);
                bool is_observe_regist_request(Resource current_resource,Address peer_addr, UString token_data, ProtocolDataUnit request);
                bool is_ct_exist(ProtocolDataUnit request_pdu);
                bool is_uscf(Resource current_resource,Address peer_addr, UString token_data, ProtocolDataUnit request);
                bool is_br(Resource current_resource,Address peer_addr, UString token_data, ProtocolDataUnit request);
                bool is_ct_valid(Resource current_resource, ProtocolDataUnit request);
            }
        }
    }
}


class BrokerResource : public Resource{
    
    public:
        /* Global variable */
        static coap_context_t *ctx_;
        static TopicDB      topic_db;
        static time_t       earliest_topic_max_age;
        static time_t		earliest_data_max_age;

        static int	compareString(char* a, char* b);
            
        static void hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
                    const coap_endpoint_t *local_interface, coap_address_t *peer, 
                    coap_pdu_t *request, str *token, coap_pdu_t *response);
    
    
        static void hnd_post_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
                    const coap_endpoint_t *local_interface, coap_address_t *peer, 
                    coap_pdu_t *request, str *token, coap_pdu_t *response);
    
        static void hnd_get_topic(coap_context_t *ctx, struct coap_resource_t *resource, 
                    const coap_endpoint_t *local_interface, coap_address_t *peer, 
                    coap_pdu_t *request, str *token, coap_pdu_t *response);
    
        static void hnd_put_topic(coap_context_t *ctx ,
                    struct coap_resource_t *resource ,
                    const coap_endpoint_t *local_interface ,
                    coap_address_t *peer ,
                    coap_pdu_t *request,
                    str *token ,
                    coap_pdu_t *response);
    
                                    
        static void hnd_post_topic(coap_context_t *ctx ,
                        struct coap_resource_t *resource ,
                        const coap_endpoint_t *local_interface ,
                        coap_address_t *peer ,
                        coap_pdu_t *request ,
                        str *token ,
                        coap_pdu_t *response );
    
        static void hnd_delete_topic(coap_context_t *ctx ,
                        struct coap_resource_t *resource ,
                        const coap_endpoint_t *local_interface ,
                        coap_address_t *peer ,
                        coap_pdu_t *request ,
                        str *token ,
                        coap_pdu_t *response );

        static void register_context(coap_context_t *ctx);
        void topic_data_ma_monitor();
        void topic_ma_monitor();

    public:

        BrokerResource(coap::UString name);
    };
}

#endif