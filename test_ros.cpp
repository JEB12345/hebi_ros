/**
 * Trying to figure out how to control Hebi's through ROS (hack)
 */

#include <stdio.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <signal.h>
#include <stdlib.h>

#include "lookup.hpp"
#include "mac_address.hpp"

/**
 * Simple helper function to print out if the group is valid, based on number of
 * modules (-1 for invalid group).
 */
void checkGroup(int num_modules)
{
  if (num_modules > 0)
    std::cout << "Found group with " << num_modules << " modules." << std::endl;
  else
    std::cout << "Group not found on network." << std::endl;
}

/**
 * Exit handler for a CTRL-C exit command
 */
void my_handler(int s)
{
  std::cout << "\nCought exit signal " << s << "\n";
  exit(1);
}

/**
 * Attempts to look for several groups on the network (hardcoded addresses below),
 * and retrieve references to them.
 */
int main()
{
  // Exit handler init
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

  // Create the lookup object
  hebi::Lookup lookup;
  
  // Wait for the module list to populate, and print out its contents
  std::this_thread::sleep_for(std::chrono::seconds(2));
  lookup.printTable();
  printf("\n");

  // Define module family and names:
  std::vector<std::string> family = {"X8-3"};

  std::vector<std::string> namesSB = {"M01","M02","M03","M04","M05","M06","M07","M08","M09","M10","M11","M12","M13","M14","M15","M16","M17","M18","M19","M20","M21","M22","M23","M24","M26"};

  // Add a last entry to kick out linear search algorithm
  namesSB.push_back("NOPE");

  // make temp vectors to store found modules
  std::vector<std::string> foundNamesSB;
  std::vector<std::string> foundNamesOther;
 
  // Create a group object
  std::unique_ptr<hebi::Group> groupSB;
  std::unique_ptr<hebi::Group> groupOther;
  std::unique_ptr<hebi::Group> groupAll;

  // NOTE: because we use unique_ptrs, this call frees any previous object and
  // takes ownership of the new one.  Memory leak prevention for the win!
  
  // Parse throught the lookup object for motors and group them according
  // to ones defined as SB and any others that might be on the network.
  std::cout << "Looking for Robot Motors.\n";
  std::unique_ptr<hebi::Lookup::EntryList> entry_list = lookup.getEntryList();
  for (int i = 0; i < entry_list->size(); ++i)
  {
    auto entry = entry_list->getEntry(i);

    // std::cout << "Name: " << entry.name_ << std::endl << "Family: " << entry.family_ << std::endl << std::endl;
    // std::cout << std::endl;
    std::string temp = entry.name_;
    //std::cout << entry.name_ << "\n";
    std::cout << temp << ": ";
    if (entry.family_.compare(family[0]) == 0)
    {
      for (std::string k : namesSB)
      {
        if (k.compare(temp) == 0)
        {
          foundNamesSB.push_back(k);
          std::cout << "Robot Motor!\n";
          break;
        }
        if (k.compare("NOPE") == 0)
        {
          foundNamesOther.push_back(temp);
          std::cout << "Not a Robot Motor.\n";
        }
      }
    }
  }

  // use a 4 second timeout.
  long timeout_ms = 4000;
  if (foundNamesSB.size() > 0)
  {
    printf("Making Robot Hebi Group.\n");
    groupSB = lookup.getGroupFromNames(foundNamesSB, family, timeout_ms);
    checkGroup(groupSB ? groupSB->size() : -1);
  }
  if (foundNamesOther.size() > 0)
  {
    printf("Making non Robot Hebi Group.\n");
    groupOther = lookup.getGroupFromNames(foundNamesOther, family, timeout_ms);
    checkGroup(groupOther ? groupOther->size() : -1);
  }

  printf("Looking up group by family.\n");
  groupAll = lookup.getGroupFromFamily(family[0], timeout_ms);
  checkGroup(groupAll ? groupAll->size() : -1);

  if (foundNamesSB.size() == 0 && foundNamesOther.size() == 0)
  {
    std::cout << "Didn't find any Motors.\n";
  }

  std::cout << "\nMain Program will now start.\n";

  while(1);

  // NOTE: destructors automatically clean up objects
  return 0;
}
