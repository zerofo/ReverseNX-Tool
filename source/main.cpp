#include <dirent.h>
#include <vector>
#include <sstream>

#include <borealis.hpp>
#include "main.hpp"
#include "FullOptionsFrame.hpp"
#include "TabOptionsFrame.hpp"

using namespace std;

Result nsError = 0x1;
std::vector<Title> titles;

uint8_t Docked[0x10] = {0xE0, 0x03, 0x00, 0x32, 0xC0, 0x03, 0x5F, 0xD6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t Handheld[0x10] = {0x00, 0x00, 0xA0, 0x52, 0xC0, 0x03, 0x5F, 0xD6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
bool dockedflag = false;
bool handheldflag = false;
bool isAlbum = false;
char Files[2][38] = { "_ZN2nn2oe18GetPerformanceModeEv.asm64", "_ZN2nn2oe16GetOperationModeEv.asm64" };
char ReverseNX[128];
uint8_t filebuffer[0x10] = {0};
NsApplicationControlData appControlData;
uint32_t countGames = 0;
Flag changeFlag = Flag_Handheld;
bool memorySafety = false;

void isFullRAM() {
	switch(appletGetAppletType()) {
		case AppletType_Application:
			break;
		
		case AppletType_SystemApplication:
			break;
		
		default:
			isAlbum = true;
			break;
	}
}

bool isAllUpper(const std::string& word)
{
	for(auto& c: word) if(std::islower(static_cast<unsigned char>(c))) return false;
	return true;
}

void RemoveReverseNX(u64 tid) {
	if (tid == UINT64_MAX) for (uint8_t i = 0; i < 2; i++) {
		snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%s", Files[i]);
		remove(ReverseNX);
	}
	else for (uint8_t i = 0; i < 2; i++) {
		snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%016" PRIx64 "/%s", tid, Files[i]);
		remove(ReverseNX);
	}
	brls::Application::notify("Found and deleted broken patches.");
}

void setReverseNX(uint64_t tid, Flag changedFlag) {
	
	for (uint8_t i = 0; i < 2; i++) {
		if (tid == UINT64_MAX) {
			snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%s", Files[i]);
			FILE* asem = fopen(ReverseNX, "wb");
			switch(changedFlag) {
				case Flag_Handheld:
					fwrite(Handheld, 1, 16, asem);
					fclose(asem);
					break;
				
				case Flag_Docked:
					fwrite(Docked, 1, 16, asem);
					fclose(asem);
					break;
				
				default:
					fclose(asem);
					remove(ReverseNX);
					break;
					
			}
		}
		else {
			snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%016" PRIx64, tid);
			DIR* patchdir = opendir(ReverseNX);
			if (patchdir == NULL) mkdir(ReverseNX, 777);
			else closedir(patchdir);
			snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%016" PRIx64 "/%s", tid, Files[i]);
			FILE* asem = fopen(ReverseNX, "wb");
			switch(changedFlag) {
				case Flag_Handheld:
					fwrite(Handheld, 1, 16, asem);
					fclose(asem);
					break;
				
				case Flag_Docked:
					fwrite(Docked, 1, 16, asem);
					fclose(asem);
					break;
				
				default:
					fclose(asem);
					remove(ReverseNX);
					break;
					
			}
		}
	}
}

Flag getReverseNX(uint64_t tid) {
	if (tid == UINT64_MAX) {
		snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%s", Files[0]);
		FILE* asem = fopen(ReverseNX, "r");
		if (asem != NULL) {
			fread(&filebuffer, 1, 16, asem);
			int c = 0;
			while (c < 16) {
				if (filebuffer[c] == Handheld[c]) c++;
				else break;
			}
			if (c != 16) {
				c = 0;
				while (c < 16) {
					if (filebuffer[c] == Docked[c]) c++;
					else break;
				}
				if (c != 16) {
					fclose(asem);
					RemoveReverseNX(tid);
					return Flag_System;
				}
				else dockedflag = true;
			}
			else handheldflag = true;
			fclose(asem);
				
			snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%s", Files[1]);
			asem = fopen(ReverseNX, "r");
			if (asem != NULL) {
				fread(&filebuffer, 1, 16, asem);
				c = 0;
				while (c < 16) {
					if (filebuffer[c] == Handheld[c]) c++;
					else break;
				}
				if (c == 16 && handheldflag == true) {
					fclose(asem);
					return Flag_Handheld;
				}
				else {
					c = 0;
					while (c < 16) {
						if (filebuffer[c] == Docked[c]) c++;
						else break;
					}
					fclose(asem);
					if (c == 16 && dockedflag == true) return Flag_Docked;
					else {
						RemoveReverseNX(tid);
						return Flag_System;
					}
				}
			}
		}
		else {
			snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%016" PRIx64 "/%s", tid, Files[1]);
			asem = fopen(ReverseNX, "r");
			if (asem != NULL) {
				fclose(asem);
				RemoveReverseNX(tid);
				return Flag_System;
			}
			else return Flag_System;
		}
	}
	else {
		snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%016" PRIx64 "/%s", tid, Files[0]);
		FILE* asem = fopen(ReverseNX, "r");
		if (asem != NULL) {
			fread(&filebuffer, 1, 16, asem);
			int c = 0;
			while (c < 16) {
				if (filebuffer[c] == Handheld[c]) c++;
				else break;
			}
			if (c != 16) {
				c = 0;
				while (c < 16) {
					if (filebuffer[c] == Docked[c]) c++;
					else break;
				}
				if (c != 16) {
					fclose(asem);
					RemoveReverseNX(tid);
					return Flag_System;
				}
				else dockedflag = true;
			}
			else handheldflag = true;
			fclose(asem);
				
			snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%016" PRIx64 "/%s", tid, Files[1]);
			asem = fopen(ReverseNX, "r");
			if (asem != NULL) {
				fread(&filebuffer, 1, 16, asem);
				c = 0;
				while (c < 16) {
					if (filebuffer[c] == Handheld[c]) c++;
					else break;
				}
				if (c == 16 && handheldflag == true) {
					fclose(asem);
					return Flag_Handheld;
				}
				else {
					c = 0;
					while (c < 16) {
						if (filebuffer[c] == Docked[c]) c++;
						else break;
					}
					fclose(asem);
					if (c == 16 && dockedflag == true) return Flag_Docked;
					else {
						RemoveReverseNX(tid);
						return Flag_System;
					}
				}
			}
		}
		else {
			snprintf(ReverseNX, sizeof ReverseNX, "sdmc:/SaltySD/patches/%016" PRIx64 "/%s", tid, Files[1]);
			asem = fopen(ReverseNX, "r");
			if (asem != NULL) {
				fclose(asem);
				RemoveReverseNX(tid);
			}
			return Flag_System;
		}
	}
		
	return Flag_System;
}

string getAppName(uint64_t Tid)
{
	size_t appControlDataSize = 0;
	NacpLanguageEntry *languageEntry = nullptr;
	Result rc;

	memset(&appControlData, 0x0, sizeof(NsApplicationControlData));

	rc = nsGetApplicationControlData(NsApplicationControlSource::NsApplicationControlSource_Storage, Tid, &appControlData, sizeof(NsApplicationControlData), &appControlDataSize);
	if (R_FAILED(rc))
	{
		stringstream ss;
		ss << 0 << hex << uppercase << Tid;
		return ss.str();
	}
	rc = nacpGetLanguageEntry(&appControlData.nacp, &languageEntry);
	if (R_FAILED(rc))
	{
		stringstream ss;
		ss << 0 << hex << uppercase << Tid;
		return ss.str();
	}
	return string(languageEntry->name);
}

vector<Title> getAllTitles()
{
	vector<Title> apps;
	NsApplicationRecord *appRecords = new NsApplicationRecord[1024]; // Nobody's going to have more than 1024 games hopefully...
	s32 actualAppRecordCnt = 0;
	Result rc;
	rc = nsListApplicationRecord(appRecords, 1024, 0, &actualAppRecordCnt);
	if (R_FAILED(rc))
	{
		while (brls::Application::mainLoop());
		exit(rc);
	}

	for (s32 i = 0; i < actualAppRecordCnt; i++)
	{
		Title title;
		title.TitleID = appRecords[i].application_id;
		title.TitleName = getAppName(appRecords[i].application_id);
		title.ReverseNX = getReverseNX(appRecords[i].application_id);
		memcpy(&title.icon, appControlData.icon, sizeof(title.icon));
		apps.push_back(title);
	}
	delete[] appRecords;
	return apps;
}

int main(int argc, char *argv[])
{
	// Init the app

	if (!brls::Application::init("ReverseNX-Tool"))
	{
		brls::Logger::error("Unable to init ReverseNX-Tool");
		return EXIT_FAILURE;
	}
	
	//Create patches folder if doesn't exist
	DIR* patches_dir = opendir("sdmc:/SaltySD/patches");
	if (patches_dir == NULL) mkdir("sdmc:/SaltySD/patches", 777);
	else closedir(patches_dir);
	
	nsError = nsInitialize();
		
	if (R_FAILED(nsError)) {
		char Error[64];
		sprintf(Error, "nsInitialize error: 0x%08x\nApplication will be closed.", (uint32_t)nsError);
		std::string stdError = Error;
		brls::Application::crash(stdError);
	}
	
	else {
		titles = getAllTitles();
		isFullRAM();

		// Add the root view to the stack
		brls::Application::pushView(new TabOptionsFrame());
	}


	// Run the app
	while (brls::Application::mainLoop());

	// Exit
	nsExit();
	return EXIT_SUCCESS;
}
