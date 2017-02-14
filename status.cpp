#include "status.h"


/* start scanner when thread is started */
SwitchmateThread status = start;

void setStatus(SwitchmateThread s){
    status = s;
}
SwitchmateThread getStatus(void){
    return status;
}


/* stores the state of the lightSwitch */
// set to -1 so when it get a reading, it's different and prints out initial state
int switchState = -1;

void setSwitchState(int state){
    switchState = state;
}
int getSwitchState(void){
    return switchState;
}


struct hci_state {
	int device_id;
	int device_handle;
	struct hci_filter original_filter;
	int state;
	int has_error;
	char error_message[1024];
} hci_state;

struct hci_state current_hci_state;

struct hci_state open_default_hci_device()
{
	struct hci_state current_hci_state = {0};

	current_hci_state.device_id = hci_get_route(NULL);

	if ((current_hci_state.device_handle = hci_open_dev(current_hci_state.device_id)) < 0)
	{
		current_hci_state.has_error = 1;
		snprintf(current_hci_state.error_message, sizeof(current_hci_state.error_message), "Could not open device: %s", strerror(errno));
		return current_hci_state;
	}

	// Set fd non-blocking
	int on = 1;
	if (ioctl(current_hci_state.device_handle, FIONBIO, (char *)&on) < 0)
	{
		current_hci_state.has_error = 1;
		snprintf(current_hci_state.error_message, sizeof(current_hci_state.error_message), "Could set device to non-blocking: %s", strerror(errno));
		return current_hci_state;
	}

	current_hci_state.state = HCI_STATE_OPEN;

	return current_hci_state;
}

void start_hci_scan(struct hci_state current_hci_state)
{
	// int hci_le_set_scan_parameters(int dev_id, uint8_t type, uint16_t interval, uint16_t window, uint8_t own_type, uint8_t filter, int to);
	if (hci_le_set_scan_parameters(current_hci_state.device_handle, 0x01, htobs(0x0010), htobs(0x0010), 0x00, 0x00, 1000) < 0)
	{
		current_hci_state.has_error = 1;
		snprintf(current_hci_state.error_message, sizeof(current_hci_state.error_message), "Failed to set scan parameters: %s", strerror(errno));
		return;
	}

	// event mask?

	// int hci_le_set_scan_enable(int dev_id, uint8_t enable, uint8_t filter_dup, int to);
	if (hci_le_set_scan_enable(current_hci_state.device_handle, 0x01, 0x00, 1000) < 0)
	{
		current_hci_state.has_error = 1;
		snprintf(current_hci_state.error_message, sizeof(current_hci_state.error_message), "Failed to enable scan: %s", strerror(errno));
		return;
	}

	current_hci_state.state = HCI_STATE_SCANNING;

	// Save the current HCI filter
	socklen_t olen = sizeof(current_hci_state.original_filter);
	if (getsockopt(current_hci_state.device_handle, SOL_HCI, HCI_FILTER, &current_hci_state.original_filter, &olen) < 0)
	{
		current_hci_state.has_error = 1;
		snprintf(current_hci_state.error_message, sizeof(current_hci_state.error_message), "Could not get socket options: %s", strerror(errno));
		return;
	}

	// Create and set the new filter
	struct hci_filter new_filter;

	hci_filter_clear(&new_filter);
	hci_filter_set_ptype(HCI_EVENT_PKT, &new_filter);
	hci_filter_set_event(EVT_LE_META_EVENT, &new_filter);

	if (setsockopt(current_hci_state.device_handle, SOL_HCI, HCI_FILTER, &new_filter, sizeof(new_filter)) < 0)
	{
		current_hci_state.has_error = 1;
		snprintf(current_hci_state.error_message, sizeof(current_hci_state.error_message), "Could not set socket options: %s", strerror(errno));
		return;
	}

	current_hci_state.state = HCI_STATE_FILTERING;
}

void stop_hci_scan(struct hci_state current_hci_state)
{
	if (current_hci_state.state == HCI_STATE_FILTERING)
	{
		current_hci_state.state = HCI_STATE_SCANNING;
		setsockopt(current_hci_state.device_handle, SOL_HCI, HCI_FILTER, &current_hci_state.original_filter, sizeof(current_hci_state.original_filter));
	}

	if (hci_le_set_scan_enable(current_hci_state.device_handle, 0x00, 1, 1000) < 0)
	{
		current_hci_state.has_error = 1;
		snprintf(current_hci_state.error_message, sizeof(current_hci_state.error_message), "Disable scan failed: %s", strerror(errno));
	}

	current_hci_state.state = HCI_STATE_OPEN;
}

void close_hci_device(struct hci_state current_hci_state)
{
	if (current_hci_state.state == HCI_STATE_OPEN)
	{
		hci_close_dev(current_hci_state.device_handle);
	}
}

void error_check_and_exit(struct hci_state current_hci_state)
{
	if (current_hci_state.has_error)
	{
		std::cout << "ERROR: " << current_hci_state.error_message << std::endl;
		exit(1);
	}
}

void process_data(uint8_t *data, size_t data_len, le_advertising_info *info)
{
	if (data[0] == SERVICE_DATA)
	{
		//std::cout << "Service data type: len=" << data_len << std::endl;

        // last bit of the 7th data piece is the Switchmate status
        setSwitchState(int(data[7]) & 1);

        /*
		int i;
		for (i = 1; i < data_len; i++)
		{
			//std::cout << "\tData : " << (int)data[i] << std::endl;
			if (i == 7) {
			    std::cout << (int(data[i]) & 1) << std::endl;
			}
		}
        */
	}
}


int get_rssi(bdaddr_t *bdaddr, struct hci_state current_hci_state)
{
	struct hci_dev_info di;
	if (hci_devinfo(current_hci_state.device_id, &di) < 0) {
		perror("Can't get device info");
		return (-1);
	}

	uint16_t handle;
// int hci_create_connection(int dd, const bdaddr_t *bdaddr, uint16_t ptype, uint16_t clkoffset, uint8_t rswitch, uint16_t *handle, int to);
// HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5
	if (hci_create_connection(current_hci_state.device_handle, bdaddr, htobs(di.pkt_type & ACL_PTYPE_MASK), 0, 0x01, &handle, 25000) < 0) {
		perror("Can't create connection");
		// TODO close(dd);
		return (-1);
	}
	sleep(1);

	struct hci_conn_info_req *cr = (hci_conn_info_req*)malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	bacpy(&cr->bdaddr, bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(current_hci_state.device_handle, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		return (-1);
	}

	int8_t rssi;
	if (hci_read_rssi(current_hci_state.device_handle, htobs(cr->conn_info->handle), &rssi, 1000) < 0) {
		perror("Read RSSI failed");
		return (-1);
	}

	printf("RSSI return value: %d\n", rssi);

	free(cr);

	usleep(10000);
	hci_disconnect(current_hci_state.device_handle, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
}

void my_handler(int s) {

	printf("Caught signal %d\n", s);

	if (s == 2) {
		stop_hci_scan(current_hci_state);

		error_check_and_exit(current_hci_state);

		close_hci_device(current_hci_state);
	}

	exit(1);
}


void scan_service(){

    struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	current_hci_state = open_default_hci_device();

	error_check_and_exit(current_hci_state);

	stop_hci_scan(current_hci_state);

	start_hci_scan(current_hci_state);

	error_check_and_exit(current_hci_state);

	std::cout << "Scanning..." << std::endl;

	int done = 0;
	int error = 0;
	while (!done && !error)
	{
		int len = 0;
		unsigned char buf[HCI_MAX_EVENT_SIZE];
		while ((len = read(current_hci_state.device_handle, buf, sizeof(buf))) < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			error = 1;
		}

		if (!done && !error)
		{
			evt_le_meta_event *meta = (evt_le_meta_event*)(((uint8_t *)&buf) + (1 + HCI_EVENT_HDR_SIZE));

			len -= (1 + HCI_EVENT_HDR_SIZE);

			if (meta->subevent != EVT_LE_ADVERTISING_REPORT)
			{
				std::cout << "continue" << std::endl;
				continue;
			}

			le_advertising_info *info = (le_advertising_info *) (meta->data + 1);

			//std::cout << "\nEvent: " << (int)info->evt_type << std::endl;
			//std::cout << "Length: " << (int)info->length << std::endl;

			if (info->length == 0)
			{
				continue;
			}

			int current_index = 0;
			int data_error = 0;

			while (!data_error && current_index < info->length)
			{
				size_t data_len = info->data[current_index];

				if (data_len + 1 > info->length)
				{
					std::cout << "EIR data length is longer than EIR packet length. " << data_len << "+ 1 > " << info->length << std::endl;
					data_error = 1;
				}
				else
				{
					process_data((uint8_t *)(((uint8_t *)&info->data) + current_index + 1), data_len, info);
					//get_rssi(&info->bdaddr, current_hci_state);
					current_index += data_len + 1;
				}
			}
		}
	}

	if (error)
	{
		std::cout << "Error scanning." << std::endl;
	}

	stop_hci_scan(current_hci_state);

	error_check_and_exit(current_hci_state);

	close_hci_device(current_hci_state);

	//return 0;

    /*
	while(1) {
        //
        if (getStatus() == running){
			// convert ASCII character in buffer to integer
			int state = buf[0] - '0';

            setSwitchState(state);
			//printf("%.*s %.*s\n", 3, buf, 3, last_buf);
			printf("The lightswitch state is now: %i\n", state);

		}

		// start scanner
		else if (getStatus() == start) {
			printf("Start status daemon\n");
			system("./Daemon/statusd.py start");

			// go back to running
			status = running;
		}

		// stop scanner
		else if (getStatus() == stop) {
			printf("Stop status daemon");
			system("./Daemon/statusd.py stop");

			// go back to running
			status = running;
		}

		// kill the thread
		else if (getStatus() == kill){
		    // TODO: give up access to the socket file location?
			pthread_exit(NULL);
		}
	}
    */
}
