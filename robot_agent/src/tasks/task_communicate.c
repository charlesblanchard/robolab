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

		int nb_stream_to_send = 43; //experimental value
		
		int save_count_pheromone;
		
		/* --- Send Victims --- */
		while(g_list_send_victim->count != 0)
		{
			nb_stream_to_send--;
			g_stat.nb_msg_sent[0]++;

			seq++;
			data = (void *)malloc(sizeof(victim_t));
			
			// Get data from the list
			doublylinkedlist_remove(g_list_send_victim, g_list_send_victim->first ,data, &data_type);
			
			//printf("Time elasped since victim detection: %f ms.\n",timelib_timer_get(g_stat.victim_event) );
			
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
		while(g_list_send_location->count != 0 && nb_stream_to_send > 0)
		{
			nb_stream_to_send--;
			g_stat.nb_msg_sent[1]++;
			
			seq++;
			data = (void *)malloc(sizeof(robot_t));
			
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
		save_count_pheromone = g_list_send_pheromone->count - 8;
		while(g_list_send_pheromone->count != 0 && g_list_send_pheromone->count != save_count_pheromone && nb_stream_to_send >0)
		{	
			nb_stream_to_send-=4;
			
			g_stat.nb_msg_sent[2]++;
			
			seq++;
			data = (void *)malloc(sizeof(pheromone_map_sector_t));
			
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
		doublylinkedlist_empty(g_list_send_pheromone);
		
		/* --- Send Stream --- */
		while(g_list_send_stream->count != 0 && nb_stream_to_send > 0)
		{
			nb_stream_to_send--;
			
			g_stat.nb_msg_sent[3]++;
			
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
		
		//doublylinkedlist_empty(g_list_send_stream);
		
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

					//count the number of go ahead
					stat_go_ahead_reception_add(&g_stat);

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
						
						if ((((command_t *)(packet.data))->cmd) == s_CMD_STOP)
						{
							// stop command received
							timelib_timer_reset(&(g_stat.stop_event));
						}
						
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
