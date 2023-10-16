/*

Base DataStructure:
   Templated Data
   Magic Number (Which will use in cache eviction policy.)

Main LRU Data Structure:

Public:
   * CTOR(capacity , eviction_policy)
   * Insert : Here we are inserting key, value pair inside internal Map. Querying 
              Capacity of the Map and Deleting / Call backing based on eviction policy.
              So here we need pointer 

              * When get call happens we need to up-date the pointer, 
              *    
   * Delete
   * Get
   
   Private:
   * Map <Key, *BaseDataStructure>

*/

