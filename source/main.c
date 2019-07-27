#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <switch.h>

int main(int argc, char **argv)
{
    consoleInit(NULL);
	DIR* flags_dir = opendir("sdmc:/SaltySD/flags");
	if (!flags_dir) {
		closedir(flags_dir);
		mkdir("sdmc:/SaltySD/flags", ACCESSPERMS);
	}
	DIR* flags_reversenx_dir = opendir("sdmc:/SaltySD/flags/ReverseNX");
	if (!flags_reversenx_dir) {
		closedir(flags_reversenx_dir);
		mkdir("sdmc:/SaltySD/flags/ReverseNX", ACCESSPERMS);
	}
	else closedir(flags_dir);
	FILE *titleid = fopen("sdmc:/SaltySD/flags/ReverseNX/titleid.flag", "r");
	
	if (titleid) {
		printf("Detected titleid.flag.\n");
		goto titleid_1;
	}
	else goto global_1;

titleid_1:
	printf("Titleid mode set. Press + to change mode to global.\n\n");
	FILE *handheld_flag_titleid = fopen("sdmc:/SaltySD/flags/ReverseNX/createhandheld.flag", "r");
	FILE *docked_flag_titleid = fopen("sdmc:/SaltySD/flags/ReverseNX/createdock.flag", "r");
	FILE *remove_flag_titleid = fopen("sdmc:/SaltySD/flags/ReverseNX/createremove.flag", "r");
	u8 handheld_titleid = 0;
	u8 docked_titleid = 0;
	u8 remove_titleid = 0;

    if (docked_flag_titleid && handheld_flag_titleid) {
		printf("Detected both flags.\n");
		fclose(handheld_flag_titleid);
		fclose(docked_flag_titleid);
		remove("sdmc:/SaltySD/flags/ReverseNX/createdock.flag");
		remove("sdmc:/SaltySD/flags/ReverseNX/createhandheld.flag");
		printf("Flags deleted.\n");
	}
	else if (docked_flag_titleid) 
	{
		printf("Detected docked titleid flag.\n");
		docked_titleid = 1;
	}
	else if (handheld_flag_titleid) 
	{
		printf("Detected handheld titleid flag.\n");
		handheld_titleid = 1;
	}
	else if (remove_flag_titleid) 
	{
		printf("Detected reset settings.\n");
		remove_titleid = 1;
	}
	else printf("No change flag detected. ReverseNX-Tool disabled.\n");
	fclose(handheld_flag_titleid);
	fclose(docked_flag_titleid);
	fclose(remove_flag_titleid);
	printf("What titleid flag you want to set?\n\n");
	printf("It will be applied only to first game You will boot.\n\n");
	printf("A - Docked, B - Handheld, Y - Reset settings, X - Exit.\n\n");

    // Main loop
    while(appletMainLoop())
    {
		hidScanInput();
		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_X) break;
		else if (kDown & KEY_A) {
			if (docked_titleid == 1) printf("You have already docked flag!\n");
			else if (docked_titleid == 0) {
				if (handheld_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createhandheld.flag");
				if (remove_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createremove.flag");
				FILE *docked_file = fopen("sdmc:/SaltySD/flags/ReverseNX/createdock.flag", "w");
				fclose(docked_file);
				docked_titleid = 1;
				handheld_titleid = 0;
				remove_titleid = 0;
				printf("Docked flag set.\n");
			}
		}
		else if (kDown & KEY_B) {
			if (handheld_titleid == 1) printf("You have already handheld flag!\n");
			else if (handheld_titleid == 0) {
				if (docked_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createdock.flag");
				if (remove_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createremove.flag");
				FILE *handheld_file = fopen("sdmc:/SaltySD/flags/ReverseNX/createhandheld.flag", "w");
				fclose(handheld_file);
				handheld_titleid = 1;
				docked_titleid = 0;
				remove_titleid = 0;
				printf("Handheld flag set.\n");
			}
		}
		else if (kDown & KEY_Y) {
			if ((handheld_titleid == 0) && (docked_titleid == 0) && (remove_titleid == 1)) printf("It's already set to be resetted!\n");
			else {
				if (docked_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createdock.flag");
				if (handheld_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createhandheld.flag");
				FILE *remove_file = fopen("sdmc:/SaltySD/flags/ReverseNX/createremove.flag", "w");
				fclose(remove_file);
				handheld_titleid = 0;
				docked_titleid = 0;
				remove_titleid = 1;
				printf("Reset flag set.\n");
			}
		}
		else if (kDown & KEY_PLUS) {
			if ((handheld_titleid != 0) || (docked_titleid != 0)) {
				if (docked_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createdock.flag");
				if (handheld_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createhandheld.flag");
				if (remove_titleid == 1) remove("sdmc:/SaltySD/flags/ReverseNX/createremove.flag");
			}
			fclose(titleid);
			remove("sdmc:/SaltySD/flags/ReverseNX/titleid.flag");
			consoleExit(NULL);
			consoleInit(NULL);
			u8 global = 1;
			if (global == 1) goto global_1;
			}

        consoleUpdate(NULL);
		}
	goto close;

global_1:
	printf("Global mode set. Press + to change mode to titleid.\n\n");
	FILE *handheld_flag_global = fopen("sdmc:/SaltySD/plugins/ReverseNX/handheld.flag", "r");
	FILE *docked_flag_global = fopen("sdmc:/SaltySD/plugins/ReverseNX/docked.flag", "r");
	u8 handheld = 0;
	u8 docked = 0;

    if (docked_flag_global && handheld_flag_global) {
		printf("Detected both flags.\n");
		fclose(handheld_flag_global);
		fclose(docked_flag_global);
		remove("sdmc:/SaltySD/plugins/ReverseNX/handheld.flag");
		remove("sdmc:/SaltySD/plugins/ReverseNX/docked.flag");
		printf("Flags deleted.\n");
	}
	else if (docked_flag_global) 
	{
		printf("Detected docked global flag.\n");
		docked = 1;
	}
	else if (handheld_flag_global) 
	{
		printf("Detected handheld global flag.\n");
		handheld = 1;
	}
	else printf("No global flag detected. ReverseNX disabled.\n");
	fclose(handheld_flag_global);
	fclose(docked_flag_global);
	printf("What global flag you want to set?\n\n");
	printf("A - Docked, B - Handheld, Y - Disable, X - Exit.\n\n");

    // Main loop
    while(appletMainLoop())
    {
		hidScanInput();
		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_X) break;
		else if (kDown & KEY_A) {
			if (docked == 1) printf("You have already docked flag!\n");
			else if (docked == 0) {
				if (handheld == 1) remove("sdmc:/SaltySD/plugins/ReverseNX/handheld.flag");
				FILE *docked_file = fopen("sdmc:/SaltySD/plugins/ReverseNX/docked.flag", "w");
				fclose(docked_file);
				docked = 1;
				handheld = 0;
				printf("Docked flag set.\n");
			}
		}
		else if (kDown & KEY_B) {
			if (handheld == 1) printf("You have already handheld flag!\n");
			else if (handheld == 0) {
				if (docked == 1) remove("sdmc:/SaltySD/plugins/ReverseNX/docked.flag");
				FILE *handheld_file = fopen("sdmc:/SaltySD/plugins/ReverseNX/handheld.flag", "w");
				fclose(handheld_file);
				handheld = 1;
				docked = 0;
				printf("Handheld flag set.\n");
			}
		}
		else if (kDown & KEY_Y) {
			if ((handheld == 0) && (docked == 0)) printf("It's already disabled!\n");
			else if ((handheld != 0) || (docked != 0)) {
				if (docked == 1) remove("sdmc:/SaltySD/plugins/ReverseNX/docked.flag");
				if (handheld == 1) remove("sdmc:/SaltySD/plugins/ReverseNX/handheld.flag");
				handheld = 0;
				docked = 0;
				printf("ReverseNX disabled.\n");
			}
		}
		else if (kDown & KEY_PLUS) {
				fclose(titleid);
				fopen("sdmc:/SaltySD/flags/ReverseNX/titleid.flag", "w");
				consoleExit(NULL);
				consoleInit(NULL);
				u8 titleidu8 = 1;
				if (titleidu8 == 1) goto titleid_1;
			}

        consoleUpdate(NULL);
		
    }
	goto close;

close:
    consoleExit(NULL);
    return 0;
}