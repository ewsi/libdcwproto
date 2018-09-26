/*
* Copyright (c) 2018 Cable Television Laboratories, Inc. ("CableLabs")
*                    and others.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Created by Jon Dennis (j.dennis@cablelabs.com)
*/


#ifdef WIN32


#include <dcwproto.h>
#include <string.h>
#include <stdio.h>


int WIN32_EXPORT
dcwprotocs_read_msgtype(const unsigned char * const buf, const unsigned buflen) {
	struct dcwmsg msg;

	if (!dcwmsg_marshal(&msg, buf, buflen)) {
		return -1; /* marshal failed */
	}

	return (int)msg.id;
}

unsigned WIN32_EXPORT
dcwprotocs_serialize_join(unsigned char * const output, const unsigned outputsize, const unsigned datachan_macaddr_count, const unsigned char * const datachan_macaddrs) {
	struct dcwmsg msg;

	/* validate */
	if (datachan_macaddr_count < 1) return 0; /* need at least one data channel mac address */
	if (datachan_macaddr_count > (sizeof(msg.sta_join.data_macaddrs) / sizeof(msg.sta_join.data_macaddrs[0]))) return 0; /* too many data channel mac addresses */

	/* form the message struct */
	msg.id = DCWMSG_STA_JOIN;
	msg.sta_join.data_macaddr_count = datachan_macaddr_count;
	memcpy(msg.sta_join.data_macaddrs, datachan_macaddrs, sizeof(dcwmsg_macaddr_t) * datachan_macaddr_count);

	/* serialize it into the given buffer... */
	return dcwmsg_serialize(output, &msg, outputsize);
}

unsigned WIN32_EXPORT
dcwprotocs_serialize_unjoin(unsigned char * const output, const unsigned outputsize, const unsigned datachan_macaddr_count, const unsigned char * const datachan_macaddrs) {
	struct dcwmsg msg;

	/* validate */
	if (datachan_macaddr_count < 1) return 0; /* need at least one data channel mac address */
	if (datachan_macaddr_count > (sizeof(msg.sta_unjoin.data_macaddrs) / sizeof(msg.sta_unjoin.data_macaddrs[0]))) return 0; /* too many data channel mac addresses */

	/* form the message struct */
	msg.id = DCWMSG_STA_UNJOIN;
	msg.sta_unjoin.data_macaddr_count = datachan_macaddr_count;
	memcpy(msg.sta_unjoin.data_macaddrs, datachan_macaddrs, sizeof(dcwmsg_macaddr_t) * datachan_macaddr_count);

	/* serialize it into the given buffer... */
	return dcwmsg_serialize(output, &msg, outputsize);
}

unsigned WIN32_EXPORT
dcwprotocs_serialize_staack(unsigned char * const output, const unsigned outputsize, const unsigned bonded_datachan_count, const unsigned char * datachan_macaddrs, const char * ssids) {
	struct dcwmsg msg;
	unsigned i;
	size_t ssidlen;


	/* validate */
	if (bonded_datachan_count < 1) return 0; /* need at least one data channel bond */
	if (bonded_datachan_count > (sizeof(msg.sta_ack.bonded_data_channels) / sizeof(msg.sta_ack.bonded_data_channels[0]))) return 0; /* too many data channel bonds */

	/* form the message struct */
	msg.id = DCWMSG_STA_ACK;
	msg.sta_ack.bonded_data_channel_count = bonded_datachan_count;
	for (i = 0; i < msg.sta_ack.bonded_data_channel_count; i++) {
		memcpy(msg.sta_ack.bonded_data_channels[i].macaddr, datachan_macaddrs, sizeof(msg.sta_ack.bonded_data_channels[i].macaddr));
		datachan_macaddrs += sizeof(msg.sta_ack.bonded_data_channels[i].macaddr);

		ssidlen = strlen(ssids);
		if (ssidlen > sizeof(msg.sta_ack.bonded_data_channels[i].ssid)) {
			return 0; /* failed... given ssid is too big... */
		}
		memcpy(msg.sta_ack.bonded_data_channels[i].ssid, ssids, ssidlen);
		if (ssidlen < sizeof(msg.sta_ack.bonded_data_channels[i].ssid)) {
			/* yuck... null terminating this way is awful... */
			msg.sta_ack.bonded_data_channels[i].ssid[ssidlen] = '\0';
		}
		ssids += ssidlen + 1;
	}

	/* serialize it into the given buffer... */
	return dcwmsg_serialize(output, &msg, outputsize);
}

unsigned WIN32_EXPORT
dcwprotocs_serialize_stanack(unsigned char * const output, const unsigned outputsize, const unsigned datachan_macaddr_count, const unsigned char * datachan_macaddrs) {
	struct dcwmsg msg;

	/* validate */
	if (datachan_macaddr_count < 1) return 0; /* need at least one data channel macaddress */
	if (datachan_macaddr_count > (sizeof(msg.sta_nack.data_macaddrs) / sizeof(msg.sta_nack.data_macaddrs[0]))) return 0; /* too many data channel macaddresses */

	/* form the message struct */
	msg.id = DCWMSG_STA_NACK;
	msg.sta_nack.data_macaddr_count = datachan_macaddr_count;
	memcpy(msg.sta_nack.data_macaddrs, datachan_macaddrs, sizeof(dcwmsg_macaddr_t) * datachan_macaddr_count);

	/* serialize it into the given buffer... */
	return dcwmsg_serialize(output, &msg, outputsize);
}


int WIN32_EXPORT
dcwprotocs_marshal_acceptsta(unsigned char * ssids, unsigned ssids_size, unsigned * const ssids_count, const unsigned char * const buf, const unsigned buflen) {
	struct dcwmsg msg;
	unsigned i;
	int ssidstrlen;

	(*ssids_count) = 0;

	if (!dcwmsg_marshal(&msg, buf, buflen)) {
		return 0; /* failed */
	}

	if (msg.id != DCWMSG_AP_ACCEPT_STA) {
		return 0; /* failed */
	}

	for (i = 0; i < msg.ap_accept_sta.data_ssid_count; i++) {
		ssidstrlen = snprintf(ssids, ssids_size, "%.*s", sizeof(msg.ap_accept_sta.data_ssids[i]), msg.ap_accept_sta.data_ssids[i]);
		if (((unsigned)ssidstrlen + 1) > ssids_size) {
			/* ran out of space for SSIDs... */
			return 0; /* failed */
		}
		ssids += ssidstrlen + 1;
		ssids_size -= ssidstrlen + 1;
	}

	(*ssids_count) = msg.ap_accept_sta.data_ssid_count;
	return 1; /* success */
}


int WIN32_EXPORT
dcwprotocs_marshal_rejectsta(unsigned char * macaddrs, unsigned macaddrs_size, unsigned * const macaddrs_count, const unsigned char * const buf, const unsigned buflen) {
	struct dcwmsg msg;
	unsigned i;

	(*macaddrs_count) = 0;

	if (!dcwmsg_marshal(&msg, buf, buflen)) {
		return 0; /* failed */
	}

	if (msg.id != DCWMSG_AP_REJECT_STA) {
		return 0; /* failed */
	}

	for (i = 0; i < msg.ap_reject_sta.data_macaddr_count; i++) {
		if (macaddrs_size < sizeof(msg.ap_reject_sta.data_macaddrs[i])) {
			/* ran out of space for data channel mac addresses... */
			return 0; /* failed */
		}
		memcpy(macaddrs, msg.ap_reject_sta.data_macaddrs[i], sizeof(msg.ap_reject_sta.data_macaddrs[i]));
		macaddrs += sizeof(msg.ap_reject_sta.data_macaddrs[i]);
		macaddrs_size -= sizeof(msg.ap_reject_sta.data_macaddrs[i]);
	}

	(*macaddrs_count) = msg.ap_reject_sta.data_macaddr_count;
	return 1; /* success */
}




#endif


