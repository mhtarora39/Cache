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

              * When ever insert call happens we need to up-date the pointer pointing to least recently used. 
              * i.e let's say limit count = 5.
              * Insert {a,1} , insert {b,2}, Insert {c,3} , insert {d,4} , Insert {e,5} ---> {1,2,3,4,5} 
              * get{a} : 6, get{b} : 7, get{c} : 8, get{d} : 9  , minimum = e 
              * insert {f,6} , minimum = a
              * insert {g,7} , minimum = b
              * get{b} --> update minimum pointing to c.
              *  
   

   * Delete
   * Get
   
   Private:
   * Map <Key, *BaseDataStructure>
   * 
*/

template <typename dtype>
struct Data {
   public:
   Data(const dtype& data) : m_data()  {}
   private:
   dtype m_data;
};  

