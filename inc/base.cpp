/*

Base BufferStructure:
   Templated Buffer
   Magic Number (Which will use in cache eviction policy.)

Main LRU Buffer Structure:

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
   * Map <Key, *BaseBufferStructure>
   *
*/

/*
This class represents a doubly link list data structure.
Which help us update the queue LRU/LFU in O(1) time.
*/

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iostream>
#include <string>
#include <exception>

std::unordered_set<std::string> EVICTION_POLICIES{"LRU", /*LFU*/};


// LinkList Data Structure For Storing the data.
template <typename Dtype>
struct Buffer
{
public:
   static std::shared_ptr<Buffer<Dtype> > create(const Dtype &buffer, const std::string &id = "")
   {
      return std::shared_ptr<Buffer<Dtype> >(new Buffer<Dtype>(buffer, id));
   }
   void set_id(const std::string &id) { m_id = id; }
   const std::string &get_id() { return m_id; }
   const Dtype &buf() { return m_buffer; }

   void destroy()
   {
      delete this;
   }
   virtual ~Buffer()
   {
   }

   std::shared_ptr<Buffer<Dtype>> next;
   std::shared_ptr<Buffer<Dtype>> prev;

protected:
   Buffer(const Dtype &buffer, const std::string &id) : m_buffer(buffer),
                                                        next(nullptr),
                                                        prev(nullptr),
                                                        m_id(id)
   {
   }

private:
   Dtype m_buffer;
   std::string m_id;
 
};

/*Base Class For Eviction Policy*/
template <typename Dtype>
struct EvictionPolicy
{

public:
   virtual std::shared_ptr<Buffer<Dtype> > &get(const std::string &id) = 0;
   virtual void set(const std::string &id, std::shared_ptr<Buffer<Dtype> > &data) = 0;
   virtual void display() = 0;
};

/*LRU EViction Class*/
template <typename Dtype>
struct LRUEviction : EvictionPolicy<Dtype>
{
   using DataPtr = std::shared_ptr<Buffer<Dtype> >;
   LRUEviction(size_t cache_limit) : m_cache_limit(cache_limit), first{nullptr}, last{nullptr}
   {
      if (cache_limit < 3)
      {
         throw std::runtime_error("Minimum Cache Limit Is 3");
      }

      if (cache_limit > __INT_MAX__)
      {
         throw std::runtime_error("Max Cache Limit is exceeded : "+ std::to_string(cache_limit) + " Limit : " + std::to_string(__INT_MAX__));
      }
   };

   DataPtr &get(const std::string &id)
   {
      // TODO Exception Handling for first access and key not found.
      auto &element = m_cache[id];
      if (m_cache.size() < 2)
      {
         return element;
      }
      update_on_get(element);
      return element;
   }

   void set(const std::string &id, DataPtr &data)
   {
      m_cache[id] = data;
      data->set_id(id);
      if (m_cache.size()  == 1)
      {
         first = data;
         last = data;
         return;
      }
      update_on_set(data);
      if (m_cache.size() >= m_cache_limit)
      {
         // Delete Last , that entry should also fo from map
         auto id = last->get_id();
         last = last->prev;
         m_cache.erase(id);
         return;
      }

   }

   void display() {
      auto root = first;
      int count = 2;
      while(root && count > 0 ) {
         std::cout << root->buf() << " ";
         root = root->next;
         count--;
      }
      std::cout << " \n First " << first->buf() << " Last " << last->buf() << " \n"; 

   }

private:
  
   void update_on_get(DataPtr &data_ptr) 
   {
      // 3<->2<->1 (First->3 , Last->1)
      // get(2) -> 2<->3<->1 (first->2 , last->1)
      // get(1) -> 1<->2<->3 (first->1, last->3)

      // 1<->2
      // get(2) --> last -> 1 2<->1 fi
      
      if(data_ptr->next == nullptr) {
         last = data_ptr->prev;
      }
      update_on_set(data_ptr);
      data_ptr->prev = nullptr;
   }


   void update_on_set(DataPtr &data_ptr)
   {
      // First -> 1->nullptr
      // Last  -> 1->nullptr
      // set(2) -> 2<->1 (First -> 2 ,  Last -> 1)
      // set(3) -> 3<->2<->1 (First->3 , Last->!)
      auto prev = first; 
      first = data_ptr;
      data_ptr->next = prev;
      prev->prev = data_ptr;     
   }
   size_t m_cache_limit;
   std::unordered_map<std::string, DataPtr> m_cache;
   DataPtr first;
   DataPtr last;
   std::string last_id;
};

template <typename BufDtype>
struct Cache
{
   Cache(size_t cache_limit, const std::string &eviction_policy)
   {

      // Can be call from factory.But we are supporting only one right now so
      // directly invoking here"

      if (eviction_policy == "LRU")
      {
         m_evic_policy = std::make_shared<LRUEviction<BufDtype> >(cache_limit);
      }
      else
      {
         throw std::runtime_error("Unsupported Eviction Policy : " + eviction_policy);
      }
   }

   const BufDtype &get(const std::string &id)
   {
      return m_evic_policy->get(id)->buf();
   }

   void set(const std::string &id, BufDtype &&data)
   {
      auto data_ptr = Buffer<BufDtype>::create(data);
      m_evic_policy->set(id, data_ptr);
   }

   void display() {
      m_evic_policy->display();
   }

private:
   std::shared_ptr<EvictionPolicy <BufDtype> > m_evic_policy;
};

int main() {
   auto cache = Cache<int>(3,"LRU");
   cache.set("1",1);
   cache.set("2",2);
   cache.display();
   cache.get("2");
   cache.display();


}