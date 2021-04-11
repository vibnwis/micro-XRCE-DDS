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


#include <uxr/client/client.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define STREAM_HISTORY  8
#define BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY
#define HEALTH_INVENTORY_NUM          3

#define PART_INVENTORY_EMPTY          0
#define PART_INVENTORY_NEW_PART       1
#define PART_INVENTORY_HEARTBEAT      2

#define LIFE_NOT_RESPONSIVE           0
#define LIFE_VERY_RESPONSIVE          4


static uxrStreamId reliable_out;
static uxrStreamId reliable_in;

static uxrObjectId participant_id;
static uxrObjectId replier_id;

char *partname[HEALTH_INVENTORY_NUM] = {"ABCD", "WXYZ", "EFGH"};
char *descriptions[HEALTH_INVENTORY_NUM] = {"Light Sensor", "Motor driver", "Toggle Switch"};
char *specifications[HEALTH_INVENTORY_NUM] = {"5V", "5V", "5V"};
char *resolutions[HEALTH_INVENTORY_NUM] = {"tbd", "tbd", "tbd"};


//char *health_inv_part_reg[HEALTH_INVENTORY_NUM] = {'\0', '\0', '\0'};
char *health_inv_part_reg[HEALTH_INVENTORY_NUM] = {"", "", ""};
uint8_t health_inv_state[HEALTH_INVENTORY_NUM] = {PART_INVENTORY_EMPTY, PART_INVENTORY_EMPTY, PART_INVENTORY_EMPTY};
uint8_t health_inv_life_status[HEALTH_INVENTORY_NUM] = {LIFE_NOT_RESPONSIVE, LIFE_NOT_RESPONSIVE, LIFE_NOT_RESPONSIVE};


uint8_t match_reg_file(char *rev_partname){

 /* match a partname in the master health inventory */
    bool found = false;
    int8_t idx_pos = 255;
    int8_t i = 0;
    for (i=0; i<3; i++){
        printf("[%d] Matching rev_partnum: %s  partnum in reg %s \n", i, rev_partname, partname[i]);
        if (strlen(health_inv_part_reg[i]) == 0)
           continue;
        else if (!strcmp(rev_partname, health_inv_part_reg[i])) {
          found = true;
          idx_pos = i;
          break;
        }
    }
    return idx_pos;
}

uint8_t match_master_file(char *rev_partname){

 /* match a partname in the master health inventory */
    bool found = false;
    int8_t idx_pos = 255;
    int8_t i = 0;
    for (i=0; i<3; i++){
        printf("[%d] Matching rev_partnum: %s  partnum in mas %s \n", i, rev_partname, partname[i]);
        if (!strcmp(rev_partname, partname[i])) {
          found = true;
          idx_pos = i;
          break;
        }
    }
    return idx_pos;
}

void decrement_life_status(void){

 /* match a partname in the master health inventory */
    int8_t i = 0;
    for (i=0; i<3; i++){
        if (health_inv_life_status[i]) {
            health_inv_life_status[i]--;
        }
    }
}

void rewrite_part_status(uint8_t pos){

 /* match a partname in the master health inventory */
    if (health_inv_state[pos] == PART_INVENTORY_NEW_PART){
       health_inv_state[pos] = PART_INVENTORY_HEARTBEAT;
    }
    health_inv_life_status[pos] = LIFE_VERY_RESPONSIVE;
}

void remove_dead_parts(void){
    int8_t i = 0;
    for (i=0; i<3; i++){
        if(!health_inv_life_status[i]) {
                health_inv_state[i] = PART_INVENTORY_EMPTY;
                health_inv_part_reg[i] = '\0';
        }
    }
}


void reg_new_part(char *rev_partname){
    int8_t i = 0;
    for (i=0; i<3; i++){
        if( !health_inv_life_status[i] && strlen(health_inv_part_reg[i]) == 0) {
            health_inv_state[i] = PART_INVENTORY_NEW_PART;
            health_inv_life_status[i] = LIFE_VERY_RESPONSIVE;
            health_inv_part_reg[i] = rev_partname;
     //       strcpy(health_inv_part_reg[i], rev_partname);
        }
    }
}

uint8_t searh_slot_reg(void){

 /* match a partname in the master health inventory */
    bool found = false;
    int8_t idx_pos = 255;
    int8_t i = 0;
    for (i=0; i<3; i++){

        if (!health_inv_life_status[i] && strlen(health_inv_part_reg[i]) == 0) {
          found = true;
          idx_pos = i;
          break;
        }
    }
    return idx_pos;
}

void on_request(
        uxrSession* session,
        uxrObjectId object_id,
        uint16_t request_id,
        SampleIdentity* sample_id,
        ucdrBuffer* ub,
        uint16_t length,
        void* args)
{
    (void) object_id;
    (void) request_id;
    (void) length;


  /* receiving a request message from to the requester */

    char rev_partname[32];
    char rev_localpartname[32];

    ucdr_deserialize_string(ub, rev_partname, sizeof(rev_partname));
    ucdr_deserialize_string(ub, rev_localpartname, sizeof(rev_localpartname));

    /* match a partname in the registered health inventory */

    uint8_t pos = 255;
    if ((pos = match_reg_file(rev_partname)) != 255) {  // the partnum already registered
        printf("b4-Matched reg partnum: %s  invenotry state=%d life-status=%d\n", rev_partname, health_inv_state[pos], health_inv_life_status[pos]);
        rewrite_part_status(pos);
        printf("a5-partnum: %s  invenotry state=%d life-status=%d\n", rev_partname, health_inv_state[pos], health_inv_life_status[pos]);

    }
    else {  // the partnum has not been registered
       printf("reg file matching failed! rev_partnum: %s \n", rev_partname);

       if ((pos = match_master_file(rev_partname)) != 255) {
            printf("Matched partnum: %s  index %d\n", rev_partname, pos);
            printf("b4-Matched reg partnum: %s  invenotry state=%d life-status=%d\n", rev_partname, health_inv_state[pos], health_inv_life_status[pos]);
            reg_new_part(rev_partname);
            printf("a5-partnum: %s  invenotry state=%d life-status=%d\n", rev_partname, health_inv_state[pos], health_inv_life_status[pos]);
       }

    }

 //   printf("Request received: (%d + %d)\n", rhs, lhs);
    printf("Request received: (%s %s)\n", rev_partname, rev_localpartname);

    char reply_buffer[3*32] = {

    };

    /* sending out a response to the requester */
    ucdrBuffer reply_ub;
    ucdr_init_buffer(&reply_ub, reply_buffer, sizeof(reply_buffer));
    ucdr_serialize_string(&reply_ub, rev_partname);

    uxr_buffer_reply(session, reliable_out, replier_id, sample_id, reply_buffer, sizeof(reply_buffer));

#ifdef WIN32
    printf("Reply send: %I64u\n", (uint64_t)(rhs + lhs));
#else
   // printf("Reply send: %" PRIu64 "\n", (uint64_t)(rhs + lhs));
   printf("Reply send: %s\n", rev_partname);
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
    uint32_t key = (args == 4) ? (uint32_t)atoi(argv[3]) : 0xCCCCDDDD;

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
    uxr_set_request_callback(&session, on_request, 0);
    if (!uxr_create_session(&session))
    {
        printf("Error at init session.\n");
        return 1;
    }

    // Streams
    uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
    reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, BUFFER_SIZE,
                    STREAM_HISTORY);

    uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
    reliable_in = uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    // Create entities
    participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
    const char* participant_xml = "<dds>"
            "<participant>"
            "<rtps>"
            "<name>default_xrce_participant</name>"
            "</rtps>"
            "</participant>"
            "</dds>";
    uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0,
                    participant_xml, UXR_REPLACE);

    replier_id = uxr_object_id(0x01, UXR_REPLIER_ID);
    const char* replier_xml = "<dds>"
            "<replier profile_name=\"my_requester\""
            "service_name=\"service_name\""
            "request_type=\"request_type\""
            "reply_type=\"reply_type\">"
            "</replier>"
            "</dds>";
    uint16_t replier_req = uxr_buffer_create_replier_xml(&session, reliable_out, replier_id, participant_id,
                    replier_xml, UXR_REPLACE);

    // Send create entities message and wait its status
    uint8_t status[2];
    uint16_t requests[2] = {
        participant_req, replier_req
    };
    if (!uxr_run_session_until_all_status(&session, 1000, requests, status, 2))
    {
        printf("Error at create entities: participant: %i requester: %i\n", status[0], status[1]);
        return 1;
    }

    // Request  requests
    uxrDeliveryControl delivery_control = {
        0
    };
    delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
    uint16_t read_data_req =
            uxr_buffer_request_data(&session, reliable_out, replier_id, reliable_in, &delivery_control);

    // Read request
    bool connected = true;
    while (connected)
    {
        uint8_t read_data_status;
        connected = uxr_run_session_until_all_status(&session, UXR_TIMEOUT_INF, &read_data_req, &read_data_status, 1);
    }

    return 0;
}
