package com.mediatek.common.ppl;

interface IPplAgent {
	byte[] readControlData();
	int writeControlData(in byte[] data);
}
