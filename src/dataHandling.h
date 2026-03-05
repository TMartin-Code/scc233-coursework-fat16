// Header file that containes data handling functions
#ifndef  DATAHANDLING_H
#define DATAHANDLING_H

#include "fat16Structures.h"
#include <stdint.h>
/**
 * @brief reads directory entry and ouputs the processed data, and updates entrys long and short filenames
 * @param entry - Pointer to a structure containing current directory entry and its short and long file names.
 * @param lfnBuffer - Pointer to a string that keeps track of current entrys long file name
 * @param checksum - Pointer to uint8_t value used to compare long file name entrys checksums
 */
void readRootDir(FileEntryNames * entry, char * lfnBuffer, uint8_t * checksum);

/**
 * @brief Updates longfile name buffer with current long file name entries data
 * @param lFN - Pointer to long file name entry
 * @param buffer - Pointer to long file name entrys buffer
 */
void getLFNChars(LongFileNameEntry * lFN, char * buffer);

/**
 * @brief Checks that current entry isnt empty or the last entry in the directory.
 * @param name - Pointer to current directory entrys unprocessed name data
 * @param attribute - uin8_t value containing current entrys attribute flags
 */
int checkValidRootFile(uint8_t * name, uint8_t attributes);

/**
 * @brief Parses directory entrys filename from unprocssed data and updates pointer to the filename
 * @param directory - Current entrys data in Directory Structure format
 * @param fileName - Pointer to string that is the filename to update.
 */
void getFileName(DirectoryStructure directory, char *fileName);

/**
 * @brief Parses directory entrys attribute flags from unprocessed data, and updates pointer to attribute flags
 * @param directory - Current entrys data in Directory Structure format
 * @param attribute - Pointer to string that is the attribute flags to update.
 */
void getAttributes(DirectoryStructure directory, char *attrFlags);

/**
 * @brief Parses the time stored in the directory data, stored in a buffer
 *  Time is in format of timeArry[0] = Hour, timeArray[1] = Minutes, timeArray[2] = Seconds, & if its the creation time, timeArray[4] = Milliseconds
 * @param time - integer value where raw time data from directory is given
 * @param timeArray - Buffer to store result in
 * @param createTrue - Interger value of milliseconds data, if calculating create time else 0 and milliseconds wont be calculated
 */
void findTime(int time, int * timeArray, int createTrue);

/**
 * @brief Parses the date from data stored in the directory data, and stores in a buffer
 *  stored in buffer as dateArray[0] = Year, dateArray[1] = Month, dateArray[2] = Day.
 * @param data - Integer value containing raw date data for the directory entry
 * @param dateArray - Buffer to store calculated date values in
 */
void findDate(int date, int * dateArray);

/**
 * @brief Returns the starting cluster, as uint32_t for the given directory entry
 * @param directory - a DirectoryStructure struct for the directory to get starting cluster from
 */
uint32_t getStartCluster(DirectoryStructure directory);

/**
 * @brief Returns the cluster pointed to by the provided cluster number, as a uint16_t
 * @param fatBuffer - Buffer containing the fat entry to read cluster chains from
 * @param start - uint16_t value of the current cluster.
 */
uint16_t nextCluster(uint16_t * fatBuffer, uint16_t start);


#endif
