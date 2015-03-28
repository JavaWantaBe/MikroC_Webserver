// Deprecated functions

void task_debugMe()
{
    unsigned long ethDebug = Ethernet_MAC_MACDBGR;
    unsigned long debugStat = 0;

    if(ethDebug & (1 << 25))
        printf("25 Tx FIFO is Full\n");

    if(ethDebug & (1 << 24))
        printf("24 Tx FIFO not empty\n");

    if(ethDebug & (1 << 21))
        printf("21 Tx FIFO read status in idle state\n");

    if(ethDebug & (1 << 20))
        printf("20 Tx FIFO read status in read state\n");

    if(ethDebug & (1 << 19))
        printf("19 MAC transmitter in pause\n");

    if(ethDebug & (1 << 18))
        printf("18 MAC transmit frame controller status idle\n");

    if(ethDebug & (1 << 17))
        printf("17 MAX transmit frame controller status waiting\n");

    if(ethDebug & (1 << 16))
        printf("16 MAC MII transmit engine active\n");

    debugStat = (ethDebug & (1 << 22) | (1 << 21));

    switch (debugStat) {
    case 0:
        printf("Tx FIFO status is: idle\n");
        break;

    case 1:
        printf("Tx FIFO status is: read state\n");
        break;

    case 2:
        printf("Tx FIFO status is: waiting for TxStatus from MAC\n");
        break;

    case 3:
        printf("Tx FIFO status is: writing the received TxStatus or flushing\n");
        break;
    }

    debugStat = (ethDebug & (1 << 9) | (1 << 8));

    switch (debugStat) {
    case 0:
        printf("Tx FIFO fill level: RxFIFO empty\n");
        break;

    case 1:
        printf("Tx FIFO fill level: RxFIFO fill-level below flow-control de-activated threshold\n");
        break;

    case 2:
        printf("Tx FIFO fill level: RxFIFO fill-level above flow-control activate threshold\n");
        break;

    case 3:
        printf("Tx FIFO fill level: RxFIFO full\n");
        break;
    }

    debugStat = (ethDebug & (1 << 6) | (1 << 5));

    switch (debugStat) {
    case 0:
        printf("Rx FIFO read controller status: IDLE\n");
        break;

    case 0:
        printf("Rx FIFO read controller status: Reading frame data\n");
        break;

    case 0:
        printf("Rx FIFO read controller status: Reading frame status\n");
        break;

    case 0:
        printf("Rx FIFO read controller status: Flushing the frame data and status\n");
        break;
    }

    if(ethDebug & (1 << 4))
        printf("Rx FIFO write controller active\n");

    if(ethDebug & (1 << 0))
        printf("MAC MII receive protocol engine active\n");

    debugStat = (ethDebug & (1 << 2) | (1 << 1));

    switch (debugStat) {
    case 0:
        printf("MAC small FIFO read controller status: active\n");
        break;

    case 1:
        printf("MAC small FIFO write controller status: active\n");
        break;
    }

    if(Ethernet_DMA_DMAMFBOCR & (1 << 28)) {
        debugStat = Ethernet_DMA_DMAMFBOCR & 0xFFE00000;
        debugStat = debugStat >> 17;
        printf("Missed frames by the application: %u\n", debugStat);
    }

    if(Ethernet_DMA_DMAMFBOCR & (1 << 16)) {
        debugStat = Ethernet_DMA_DMAMFBOCR & 0xFFFF;
        printf("Missed frames by the controller: %u\n", debugStat);
    }
}

// Returns data requested from server
int getSvrData(char *request, int port)
{
    char *res, domain[DOMAINSIZE];
    unsigned char *remoteIP;
    int i = 0, j = 2;

    // if request is less than 50 chars, invalidate
    if(strlen(request) < 50)
        return -1;
    else if(strncmp(request, "http", 4) != 0)
        return -1;

    // finds the char where // starts in the request
    res = strstr(request, "//");

    // parse out domain from request
    while(res[j] != '/' && i < DOMAINSIZE)
        domain[i++] = res[j++];

    domain[i] = '\0';  // Should be opendap.co-ops.nos.noaa.gov

    // allocate some memory if clientRequest is 0
    if(globalSocket_t.clientRequest == 0) {
        //globalSocket_t.clientRequest = (char*)Malloc(CLIENTREQUESTSIZE * sizeof(char));
    }

    // if memory error occured, clientRequest is 0
    if(globalSocket_t.clientRequest == 0)
        return -1;

    memset(globalSocket_t.clientRequest, 0, CLIENTREQUESTSIZE);
    // find the domain in the request
    res = strstr(request, domain);
    res += strlen(domain);
    strcpy(globalSocket_t.clientRequest, "GET ");
    strcat(globalSocket_t.clientRequest, res);
    strcat(globalSocket_t.clientRequest, " HTTP/1.1\nHost: ");
    strcat(globalSocket_t.clientRequest, domain);
    strcat(globalSocket_t.clientRequest, "\nAccept: */*\n\n");

    // request string assembled and ready to connect
    if((remoteIP = Net_Ethernet_dnsResolve(domain, 5)) != 0) {
        unsigned char *serverIpAddr[4];
        j = 10;
        memcpy(serverIpAddr, remoteIP, 4);  // Translates domain via dns

        // to keep up from a infinite loop, 10 tries are attempted by variable j
        while(i != 1 && j--!= 0) {
            i = Net_Ethernet_connectTCP(serverIpAddr, port, REQUESTPORT, &globalSocket_t.socket_global);
        }

        // set the request flag and request mark
        globalSocket_t.requestFlag = 1;
        globalSocket_t.request_mark = 1;
        return 0;
    } else {
        // if error happened then free the memory
        globalSocket_t.clientRequest = 0;
        return -1;
    }
}