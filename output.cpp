
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>

std::ofstream ofs;
std::wofstream ofsW;

const char * outputFileName = "C:\\Users\\danish\\Desktop\\GTASADLL_msgs.txt";

std::ofstream getOutputLogStream()
{
	std::ofstream ofs;

	ofs.open ( outputFileName, std::ofstream::out | std::ofstream::app);

	return ofs;
}

std::wofstream getOutputLogStreamW()
{
	std::wofstream ofsW;

	ofsW.open  (outputFileName, std::wofstream::out | std::wofstream::app);

	return ofsW;
}


void closeOutputStreams()
{
	ofsW << "---------------------OUTPUT STREAMS CLOSING---------------------------------------  " << std::endl;

	ofs.close();
	ofsW.close();
}

void PrepareOutputStreams()
{

	ofs = getOutputLogStream();
	ofsW = getOutputLogStreamW();

	ofsW << "Testing testing.....  " << std::endl;


}
