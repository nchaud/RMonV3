///* Can return any HTTP status code. Must return 0 ONLY  on success. */
//uint16_t GsmManager::sendViaGprs(const char* data)
//{
	//if (_isMock)
	//{
		//RM_LOG2(F("Mocking GPRS-Send"),data);
		////MOCK_DATA_SENT_GPRS = data;
		//return 0;
	//}
	//
	//RM_LOG2(F("Sending Actual data via GPRS"),data);
//
	//uint16_t ret = 1;
	//
	//// Post data to website
	//uint16_t statuscode;
	//int16_t length;
	//char* url="http://cars.khuddam.org.uk/r.php"; //TODO: check should not require https!
//
	//boolean succ =
		//fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length);
		//
	////Try alternative APN settings - TODO: store these elsewhere for easy testing/configuration on-site
	//if (!succ) {
		//fona.setGPRSNetworkSettings(F("web.mtn.ci"), F(""), F(""));
		//succ = fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length);
	//}
//
	//if (!succ)
	//{
		//ret = 999;
	//}
	//else
	//{
		//RM_LOG2(F("GPRS Status:"), statuscode);
			                                                      //
		//while (length > 0) {
			//while (fona.available()) {
				//char c = fona.read();
					                                                      //
				//loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
				//UDR0 = c;
					                                                      //
				//length--;
				//if (! length) break;
			//}
		//}
		//fona.HTTP_POST_end();
			                                                      //
		////200 => Success
		//ret = (statuscode == 200) ? 0 : statuscode;
	//}
	                                                      //
	//return ret;
//}
