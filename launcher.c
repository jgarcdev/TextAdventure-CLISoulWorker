#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <curl/curl.h>



#define BAD_EXIT 0xff // -1

#define INSTALLER "clisw-installer"
#define LAUNCHER "clisw-launcher"
#define GAME "clisw"

#define INSTALLER_DIR ".."
#define DATA_DIR "data"
#define MISC_DIR "data/misc"
#define STORY_DIR "data/story"
#define MAPS_DIR "data/maps"
#define SAVES_DIR "data/saves"


#define UNREACHABLE() do { printf("SHOULD NOT REACH THIS!\n"); exit(-1); } while (0);


enum OPTS {
  FIX,
  UPDATE,
  NONE
};

/**
 * Wrapper around sleep function for Windows and Linux.
 * @param ms Amount of milliseconds to sleep for
 */
static void ssleep(int ms) {
  usleep(ms * 1000);
}



size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  size_t realsize = size * nmemb;
  strncat((char*)userp, (char*)contents, realsize);
  return realsize;
}

bool checkLatest() {
  CURL* curl;
  CURLcode res;
  char remoteVersion[256] = {0};
  
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/vnd.github.raw+json");
    headers = curl_slist_append(headers, "User-Agent: VersionChecker/1.0");

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/jgarcdev/TextAdventure-CLISoulWorker/contents/version");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // Follow redirects
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);       // Max 5 redirects
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, remoteVersion);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      curl_easy_cleanup(curl);
      curl_slist_free_all(headers);
      curl_global_cleanup();
      return -1;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }
  curl_global_cleanup();

  // Read local version
  FILE* fp = fopen("version", "r");
  if (!fp) return -1;

  char localVersion[256] = {0};
  fgets(localVersion, sizeof(localVersion), fp);
  fclose(fp);

  // Trim newlines
  localVersion[strcspn(localVersion, "\r\n")] = 0;
  remoteVersion[strcspn(remoteVersion, "\r\n")] = 0;

  printf("Checking version...\n");
  printf("Current version is %s\n", localVersion);
  printf("Latest version is %s\n", remoteVersion);

  if (strcmp(remoteVersion, localVersion) == 0) return true;
  return false;
}

/**
 * Runs the installer based on the passed options.
 * @param opt The option for the installer
 */
static void runInstaller(enum OPTS opt) {
  printf("Changing to installer directory...\n");
  ssleep(1000);

  int ret = chdir(INSTALLER_DIR);

  if (ret == -1) {
    perror("ERROR");
    printf("Cannot change into installer directory. Exiting...\n");
    exit(-1);
  }

  char* args[3] = {INSTALLER, NULL, NULL};

  if (opt == FIX) args[1] = "-f";
  else args[1] = "-u";

  int execRet = execv(INSTALLER, args);

  if (execRet == -1) {
    perror("ERROR");
    printf("Could not execute installer!\n");
    exit(BAD_EXIT);
  }
}

int main(int argc, char const* argv[]) {

  // check existance of version file
  FILE* versionFile = fopen("version", "r");
  if (!versionFile) {
    printf("Could not find version file! Fixing files...\n");

    runInstaller(FIX);
    UNREACHABLE()
  }
  fclose(versionFile);

  ssleep(1000);

  bool latest = checkLatest();
  if (!latest) {
    printf("New version found! Do you want to update? [y|n] \n");
    char response = tolower(getchar());

    if (response == 'y') {
      runInstaller(UPDATE);
      UNREACHABLE()
    } else printf("Will not update. It is highly recommended to update for latest features and fixes!\n");
  }

  // will not update, now verify file integrity
  // for future, do it in a better way
  printf("Verifying files and data...\n");
  ssleep(1000);

  FILE* game = fopen(GAME, "r");
  if (!game) {
    printf("Could not find game! Fixing file...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  fclose(game);


  DIR* data = opendir(DATA_DIR);
  if (!data) {
    printf("Could not find data! Reinstalling...\n");
    runInstaller(UPDATE);
    UNREACHABLE()
  }
  closedir(data);

  DIR* maps = opendir(MAPS_DIR);
  if (!maps) {
    printf("Could not find map data! Fixing files...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  closedir(maps);

  DIR* misc = opendir(MISC_DIR);
  if (!misc) {
    printf("Could not find misc data! Fixing files...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  closedir(misc);

  DIR* story = opendir(STORY_DIR);
  if (!story) {
    printf("Could not find story data! Fixing files...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  closedir(story);

  // story = opendir("data/story/best_showtime");
  // if (!story) {
  //   printf("Could not find story data! Fixing files...\n");
  //   runInstaller(FIX);
  //   UNREACHABLE()
  // }
  // closedir(story);

  story = opendir("data/story/control_zone");
  if (!story) {
    printf("Could not find story data! Fixing files...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  closedir(story);

  story = opendir("data/story/r_square");
  if (!story) {
    printf("Could not find story data! Fixing files...\n");
    runInstaller(FIX);
    UNREACHABLE()
  }
  closedir(story);

  // story = opendir("data/story/tower_of_greed");
  // if (!story) {
  //   printf("Could not find story data! Fixing files...\n");
  //   runInstaller(FIX);
  //   UNREACHABLE()
  // }
  // closedir(story);

  int ret;

  DIR* saves = opendir(SAVES_DIR);
  if (!saves) {
    printf("Could not find save data directory! Creating saves directory...\n");

    ret = mkdir(SAVES_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    if (ret == -1) {
      if (errno != EEXIST) {
        perror("ERROR");
        printf("Cannot create saves directory. Exiting...\n");
        exit(-1);
      }
    }
  }
  closedir(saves);

  printf("Verification done. Game starting...\n");
  ssleep(1000);

  const char* launcherFlag = "-l";
  int execRet = execl(GAME, GAME, launcherFlag, NULL);
  if (execRet == -1) {
    printf("COULD NOT EXECUTE CLISW, ABORTING\n");
    exit(-1);
  }

  return 0;
}