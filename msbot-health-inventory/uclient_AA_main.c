// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
How to know which kind of Topic has been received?
To know which kind of Topic has been received, we can use the object_id parameter or
the request_id. The id of the object_id corresponds to the DataReader that has read
the Topic, so it can be useful to discretize among different topics.
*/
#include "HelloWorld.h"

#include <uxr/client/client.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include "usbgpio8.h"

#define STREAM_HISTORY  8
#define BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY

char partname[] = "ABCD";
char localpartname[] = "ABCD-local";
char specifications[] = "5V";

void on_topic(
        uxrSession* session,
        uxrObjectId object_id,
        uint16_t request_id,
        uxrStreamId stream_id,
        struct ucdrBuffer* ub,
        uint16_t length,
        void* args)
{
    (void) session; (void) object_id; (void) request_id; (void) stream_id; (void) length;

    HelloWorld topic;
    HelloWorld_deserialize_topic(ub, &topic);

    char key[20];
    snprintf(key, 20, "0x%X%X%X%X", session->info.key[0], session->info.key[1], session->info.key[2],
            session->info.key[3]);
    printf("Session %s: %s (%i)\n", key, topic.message, topic.index);
}

void on_reply(
        uxrSession* session,
        uxrObjectId object_id,
        uint16_t request_id,
        uint16_t reply_id,
        ucdrBuffer* ub,
        uint16_t length,
        void* args)
{
    (void) object_id;
    (void) request_id;
    (void) length;

    //uint64_t result;
    char result[3*32];
    ucdr_deserialize_string(ub, result, sizeof(result));

#ifdef WIN32
    printf("Reply received: %I64u [id: %d]\n", result, reply_id);
#else
    //printf("Reply received: %" PRIu64 " [id: %d]\n", result, reply_id);
    printf("Reply received: %s [id: %d]\n", result, reply_id);
#endif /* ifdef WIN32 */
}



int main(
        int args,
        char** argv)
{
    if (3 > args || 0 == atoi(argv[2]))
    {
        printf("usage: program [-h | --help] | ip port [key]\n");
        return 0;
    }

    char* ip = argv[1];
    char* port = argv[2];
    uint32_t key = (args == 4) ? (uint32_t)atoi(argv[3]) : 0xAAAABBBB;

    // Transport
    uxrUDPTransport transport;
    if (!uxr_init_udp_transport(&transport, UXR_IPv4, ip, port))
    {
        printf("Error at init transport.\n");
        return 1;
    }

    // Session
    uxrSession session;
    uxr_init_session(&session, &transport.comm, key);
    uxr_set_reply_callback(&session, on_reply, false);
    uxr_set_topic_callback(&session, on_topic, NULL);
    if (!uxr_create_session(&session))
    {
        printf("Error at init session.\n");
        return 1;
    }

    // Streams
    uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
    uxrStreamId reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, BUFFER_SIZE,
                    STREAM_HISTORY);

    uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
    uxrStreamId reliable_in = uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer, BUFFER_SIZE,
                    STREAM_HISTORY);

    // Create entities
    uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
    const char* participant_xml = "<dds>"
            "<participant>"
            "<rtps>"
            "<name>default_xrce_participant</name>"
            "</rtps>"
            "</participant>"
            "</dds>";
    uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0,
                    participant_xml, UXR_REPLACE);

    uxrObjectId requester_id = uxr_object_id(0x01, UXR_REQUESTER_ID);
    const char* requester_xml = "<dds>"
            "<requester profile_name=\"my_requester\""
            "service_name=\"service_name\""
            "request_type=\"request_type\""
            "reply_type=\"reply_type\">"
            "</requester>"
            "</dds>";
    uint16_t requester_req = uxr_buffer_create_requester_xml(&session, reliable_out, requester_id, participant_id,
                    requester_xml, UXR_REPLACE);

    //-------- publisher and subscriber --------

    uxrObjectId topic_id_1_1 = uxr_object_id(0x01, UXR_TOPIC_ID);
    const char* topic_xml_1_1 = "<dds>"
            "<topic>"
            "<name>HelloWorldTopic_1_to_2</name>"
            "<dataType>HelloWorld</dataType>"
            "</topic>"
            "</dds>";
    uint16_t topic_req_1_1 = uxr_buffer_create_topic_xml(&session, reliable_out, topic_id_1_1, participant_id,
                    topic_xml_1_1, UXR_REPLACE);

    uxrObjectId topic_id_1_2 = uxr_object_id(0x02, UXR_TOPIC_ID);
    const char* topic_xml_1_2 = "<dds>"
            "<topic>"
            "<name>HelloWorldTopic_2_to_1</name>"
            "<dataType>HelloWorld</dataType>"
            "</topic>"
            "</dds>";
    uint16_t topic_req_1_2 = uxr_buffer_create_topic_xml(&session, reliable_out, topic_id_1_2, participant_id,
                    topic_xml_1_2, UXR_REPLACE);

    uxrObjectId publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
    const char* publisher_xml = "";
    uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session, reliable_out, publisher_id,
                    participant_id, publisher_xml, UXR_REPLACE);

    uxrObjectId datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
    const char* datawriter_xml = "<dds>"
            "<data_writer>"
            "<topic>"
            "<kind>NO_KEY</kind>"
            "<name>HelloWorldTopic_1_to_2</name>"
            "<dataType>HelloWorld</dataType>"
            "</topic>"
            "</data_writer>"
            "</dds>";
    uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session, reliable_out, datawriter_id,
                    publisher_id, datawriter_xml, UXR_REPLACE);

    uxrObjectId subscriber_id = uxr_object_id(0x01, UXR_SUBSCRIBER_ID);
    const char* subscriber_xml = "";
    uint16_t subscriber_req = uxr_buffer_create_subscriber_xml(&session, reliable_out, subscriber_id,
                    participant_id, subscriber_xml, UXR_REPLACE);

    uxrObjectId datareader_id = uxr_object_id(0x01, UXR_DATAREADER_ID);
    const char* datareader_xml = "<dds>"
            "<data_reader>"
            "<topic>"
            "<kind>NO_KEY</kind>"
            "<name>HelloWorldTopic_2_to_1</name>"
            "<dataType>HelloWorld</dataType>"
            "</topic>"
            "</data_reader>"
            "</dds>";
    uint16_t datareader_req = uxr_buffer_create_datareader_xml(&session, reliable_out, datareader_id,
                    subscriber_id, datareader_xml, UXR_REPLACE);

    //---------- end PnS -------------------

    // Send create entities message and wait its status

    uint16_t requests[] = {
        participant_req, requester_req, topic_req_1_1, topic_req_1_2, publisher_req, datawriter_req, subscriber_req,
        datareader_req
    };
    uint8_t req_status[sizeof(requests) / 2];

    if (!uxr_run_session_until_all_status(&session, 1000, requests, req_status, 2))
    {
        printf("Error at create entities: participant: %i requester: %i\n", req_status[0], req_status[1]);
        return 1;
    }

    // Request replies
    uxrDeliveryControl delivery_control = {
        0
    };
    delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
    uint16_t read_data_req = uxr_buffer_request_data(&session, reliable_out, requester_id, reliable_in,
                    &delivery_control);

    // setup NUMATO USBGPIO8
    setup_USBGPIO8();

    //Set GPIO 0-3 input
    iodir_gpio_input(0);
    iodir_gpio_input(1);
    iodir_gpio_input(2);
    iodir_gpio_input(3);

     //Set GPIO 4-7 as output
    iodir_gpio_output(4);
    iodir_gpio_output(5);
    iodir_gpio_output(6);
    iodir_gpio_output(7);

    bool toggle = true;
    write_gpio_output(toggle);
    sleep(3);

    // Write requests
    bool connected = true;
    uint32_t count = 0;
    while (connected)
    {
      //uint8_t request[32 * 3] = {
        char request[32 * 3] = {  /* 3 strings  of 32 chars long */
             0
        };

        ucdrBuffer ub;

        ucdr_init_buffer(&ub, request, sizeof(request));
    //    ucdr_serialize_uint32_t(&ub, count);
    //    ucdr_serialize_uint32_t(&ub, count);

    //    ucdr_serialize_uint32_t(&ub, &partname);
    //    ucdr_serialize_uint32_t(&ub, &localpartname);

        ucdr_serialize_string(&ub, partname);
        ucdr_serialize_string(&ub, localpartname);

        uint16_t request_id = uxr_buffer_request(&session, reliable_out, requester_id, request, sizeof(request));
        //printf("Request sent: (%d + %d) [id: %d]\n", count, count, request_id);
        printf("Request sent: (%s + %s) [id: %d]\n", partname, localpartname, request_id);
        connected = uxr_run_session_time(&session, 1000);

        ++count;
    }

    remove_USBGPIO8();
    return 0;
}
