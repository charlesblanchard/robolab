/**
 * @file	task_communicate.c
 * @author  Eriks Zaharans
 * @date    31 Oct 2013
 *
 * @section DESCRIPTION
 *
 * Communicate Task.
 */

/* -- Includes -- */
/* system libraries */

/* project libraries */
#include "task.h"

#define NB_MESSAGES_MAX 100

 /**
 * Communication (receive and send data)
 */
void task_communicate(void)
{
	// Check if task is enabled
	if(g_task_communicate.enabled == s_TRUE)
	{
		// Loacal variables
		void *data; // Void pointer for data
		int data_type; // Data type

		// UDP Packet
		char udp_packet[g_config.udp_packet_size];
		int udp_packet_len;

		// Protocol
		protocol_t packet;

		//Start the new sequence
		int seq = 0; // Massi thing
		//In principle I want to send all the data in the buffer
		int last_id = g_list_send->count; // Massi thing


		// --------------------------------------------------
		//	LAB 2 starts here
		// --------------------------------------------------
/*
victim					report task	
location(latest) 		control task
pheromone 				navigate task
stream					mission task
*/
		
		unsigned int sent = 0;
		while(sent < NB_MESSAGES_MAX){
			
			
		
		
		}
		
		
		/* --- Send Victims --- */
		while(g_list_send_victim->count != 0)
		{
			if( sent > NB_MESSAGES_MAX )
			{
				// on fait quoi si on a pas tout envoyer ?
				doublylinkedlist_empty(g_list_send_victim);
				break;
			}
			
			sent++;
			seq++;
			
			data = (void *)malloc(sizeof(victim_t));
			
			// Get data from the list
			doublylinkedlist_remove(g_list_send_victim, g_list_send_victim->first ,data, &data_type);

			// Encode data into UDP packet
			protocol_encode(udp_packet,
					&udp_packet_len,
					s_PROTOCOL_ADDR_BROADCAST,
					g_config.robot_id,
					g_config.robot_team,
					s_PROTOCOL_TYPE_DATA,
					seq,
					g_message_sequence_id,
					last_id,
					data_type,
					data);

			// Broadcast packet
			udp_broadcast(g_udps, udp_packet, udp_packet_len);

			// Free memory
			free(data);
		}
		
		/* --- Send Location --- */
		while(g_list_send_location->count != 0)
		{
			if( sent > NB_MESSAGES_MAX )
			{
				break;
			}
			
			sent++;
			seq++;
			
			data = (void *)malloc(sizeof(victim_t));
			
			// Get data from the list
			doublylinkedlist_remove(g_list_send_location, g_list_send_location->first ,data, &data_type);

			// Encode data into UDP packet
			protocol_encode(udp_packet,
					&udp_packet_len,
					s_PROTOCOL_ADDR_BROADCAST,
					g_config.robot_id,
					g_config.robot_team,
					s_PROTOCOL_TYPE_DATA,
					seq,
					g_message_sequence_id,
					last_id,
					data_type,
					data);

			// Broadcast packet
			udp_broadcast(g_udps, udp_packet, udp_packet_len);

			// Free memory
			free(data);
			
			doublylinkedlist_empty(g_list_send_location);
		}
		
		/* --- Send Pheromones --- */
		while(g_list_send_pheromone->count != 0)
		{
			if( sent > NB_MESSAGES_MAX )
			{
				doublylinkedlist_empty(g_list_send_pheromone);
				break;
			}
			
			sent++;
			seq++;
			
			data = (void *)malloc(sizeof(pheromone_t));
			
			// Get data from the list
			doublylinkedlist_remove(g_list_send_pheromone, g_list_send_pheromone->first ,data, &data_type);

			// Encode data into UDP packet
			protocol_encode(udp_packet,
					&udp_packet_len,
					s_PROTOCOL_ADDR_BROADCAST,
					g_config.robot_id,
					g_config.robot_team,
					s_PROTOCOL_TYPE_DATA,
					seq,
					g_message_sequence_id,
					last_id,
					data_type,
					data);

			// Broadcast packet
			udp_broadcast(g_udps, udp_packet, udp_packet_len);

			// Free memory
			free(data);
		}
		
		/* --- Send Stream --- */
		while(g_list_send_stream->count != 0)
		{
			if( sent > NB_MESSAGES_MAX )
			{
				doublylinkedlist_empty(g_list_send_stream);
				break;
			}
			
			sent++;
			seq++;
			
			data = (void *)malloc(sizeof(stream_t));
			
			// Get data from the list
			doublylinkedlist_remove(g_list_send_stream, g_list_send_stream->first ,data, &data_type);

			// Encode data into UDP packet
			protocol_encode(udp_packet,
					&udp_packet_len,
					s_PROTOCOL_ADDR_BROADCAST,
					g_config.robot_id,
					g_config.robot_team,
					s_PROTOCOL_TYPE_DATA,
					seq,
					g_message_sequence_id,
					last_id,
					data_type,
					data);

			// Broadcast packet
			udp_broadcast(g_udps, udp_packet, udp_packet_len);

			// Free memory
			free(data);
		}



		/* --- Receive Data --- */
		// Receive packets, decode and forward to proper process

		// Receive UDP packet
		while(udp_receive(g_udps, udp_packet, &udp_packet_len) == s_OK)
		{
			// Decode packet
			//printf("%s\n",udp_packet);
			if(protocol_decode(&packet, udp_packet, udp_packet_len, g_config.robot_id, g_config.robot_team) == s_OK)
			{
				// Now decoding depends on the type of the packet
				switch(packet.type)
				{
				// ACK
				case s_PROTOCOL_TYPE_ACK :
					// Do nothing
					break;

				//Massi: go_ahead packet
				case s_PROTOCOL_TYPE_GO_AHEAD :
				{
					// Declare go ahead command 
					command_t go_ahead;
					go_ahead.cmd = s_CMD_GO_AHEAD;
					// Redirect to mission by adding it to the queue
					queue_enqueue(g_queue_mission, &go_ahead, s_DATA_STRUCT_TYPE_CMD);

					// Debuging stuff
					debug_printf("GO_AHEAD RECEIVED for robot %d team %d\n",packet.recv_id,packet.send_team);
					// Calculate time from packet (ms and s)
					int send_time_s = floor(packet.send_time / 1000);
					int send_time_ms = packet.send_time % 1000;
					int now = floor(((long long)timelib_unix_timestamp() % 60000) / 1000);
					debug_printf("GO_AHEAD_TIME: %d (%d)\n",send_time_s,now);

					break;
				}
				// Data
				case s_PROTOCOL_TYPE_DATA :
					// Continue depending on the data type
					switch(packet.data_type)
					{
					// Robot pose
					case s_DATA_STRUCT_TYPE_ROBOT :
						debug_printf("received robot\n");
						// Do nothing
						break;
					// Victim information
					case s_DATA_STRUCT_TYPE_VICTIM :
						debug_printf("received victim\n");
						// Redirect to mission by adding it to the queue
						queue_enqueue(g_queue_mission, packet.data, s_DATA_STRUCT_TYPE_VICTIM);
						break;
					// Pheromone map
					case s_DATA_STRUCT_TYPE_PHEROMONE :
						debug_printf("received pheromone\n");
						// Redirect to navigate by adding it to the queue
						queue_enqueue(g_queue_navigate, packet.data, s_DATA_STRUCT_TYPE_PHEROMONE);

						break;
					// Command
					case s_DATA_STRUCT_TYPE_CMD :
						debug_printf("received CMD\n");
						// Redirect to mission by adding it to the queue
						queue_enqueue(g_queue_mission, packet.data, s_DATA_STRUCT_TYPE_CMD);
						break;
					case s_DATA_STRUCT_TYPE_STREAM :
						debug_printf("received data stream item\n");
						break;
					// Other
					default :
						// Do nothing
						break;
					}
				// Other ?
				default:
					// Do nothing
					break;
				}

				// Free memory (only if data packet was received!)
				if(packet.type == s_PROTOCOL_TYPE_DATA)
					free(packet.data);
			}
		}

		// Increase msg sequence id
		g_message_sequence_id++;
	}
}
