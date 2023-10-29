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

#define LOG_ERROR std::cout << "[ERROR] : "
#define LOG_WARNING std::cout << "[WARNING] : "
#define LOG_INFO std::cout << "[INFO] : "

#define TRY_RELEASE(FUNC, MSG)    \
   try                            \
   {                              \
      FUNC;                       \
   }                              \
   catch (...)                    \
   {                              \
      LOG_WARNING << MSG << "\n"; \
   }

std::unordered_set<std::string> EVICTION_POLICIES{"LRU", /*LFU*/};

// LinkList Data Structure For Storing the data.
template <typename Dtype>
struct Buffer
{
public:
   static std::shared_ptr<Buffer<Dtype> > create(const Dtype &buffer, const std::string &id = "")
   {
      return std::make_shared<Buffer<Dtype> >(buffer, id);
   }
   void set_id(const std::string &id) { m_id = id; }
   const std::string &get_id() { return m_id; }
   const Dtype &buf() { return m_buffer; }

   virtual ~Buffer()
   {
   }

   void update_buf(const Dtype &buffer)
   {
      m_buffer = buffer;
   }

   std::shared_ptr<Buffer<Dtype> > next;
   std::shared_ptr<Buffer<Dtype> > prev;
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
         throw std::runtime_error("Max Cache Limit is exceeded : " + std::to_string(cache_limit) + " Limit : " + std::to_string(__INT_MAX__));
      }
   };

   DataPtr &get(const std::string &id)
   {
      // TODO Exception Handling for first access and key not found.
      if (m_cache.find(id) == m_cache.end())
      {
         throw std::runtime_error("Key Not Found!");
      }
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

      // Updating exiting key , Moving at the top of queue.
      if (m_cache.find(id) != m_cache.end())
      {
         auto &prev_ele = m_cache[id];
         prev_ele->update_buf(data->buf());
         update_on_get(prev_ele);
         return;
      }
      else
      {
         m_cache[id] = data;
         data->set_id(id);
      }
      // First Element
      if (m_cache.size() == 1)
      {
         first = data;
         last = data;
         return;
      }

      // Moving forward removing element if cache limit is exceeded.
      update_on_set(data);
      if (m_cache.size() > m_cache_limit)
      {
         // Delete Last , that entry should also fo from map
         auto id = last->get_id();
         last = last->prev;
         last->next = nullptr;
         m_cache.erase(id);
         return;
      }
   }

   void display()
   {
      auto root = first;
      while (root /*&& count > 0*/)
      {

         auto get_buf = [](const DataPtr &dptr)
         {
            return dptr == nullptr ? "NULLPTR" : std::to_string(dptr->buf());
         };

         std::cout << root->buf() << /*"( PREV : " << get_buf(root->prev) <<  " NEXT : " << get_buf(root->next) << " )*/ " <-> ";
         root = root->next;
      }
      std::cout << "\nFirst " << first->buf() << " Last " << last->buf() << " \n";
   }

private:
   // Moving current data_ptr to top of the queue. Rest of the order unchanged.
   void update_on_get(DataPtr &data_ptr)
   {
      if (first->get_id() == data_ptr->get_id())
      {
         return;
      }
      auto prev = data_ptr->prev;
      auto next = data_ptr->next;

      data_ptr->prev = nullptr;

      prev->next = next;
      if (next == nullptr)
      {
         last = prev;
      }
      else
      {
         next->prev = prev;
      }
      data_ptr->next = first;
      first->prev = data_ptr;
      first = data_ptr;
   }

   // Updating the element to top of the queue as it recently added.
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

// Cache Object take the data type we want to store. This class also taking eviction policy
// and limit to cache, Providing get and set APIs of the cache
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

   void display()
   {
      m_evic_policy->display();
   }

private:
   std::shared_ptr<EvictionPolicy<BufDtype> > m_evic_policy;
};

int main()
{
   auto cache = Cache<int>(3, "LRU");
   std::cout << "Cache limit = 3 , Adding 1,2,3,4 in cache!\n";
   cache.set("1", 1);
   cache.set("2", 2);
   cache.set("3", 3);
   cache.set("4", 4);
   cache.display();
   std::cout << "Output Should be 4<->3<->2\n";
   cache.get("2");
   std::cout << "Getting 2 from cache\n";
   cache.display();
   std::cout << "Order should be 2<->4<->3\n";
   std::cout << "Getting 1 from cache Should Throw Error\n ";
   TRY_RELEASE(cache.get("1"), "KEY NOT FOUND");
   cache.display();
   std::cout << "Order should be 2<->4<->3\n";
   std::cout << "Updating 4 to 5\n";
   cache.set("4", 5);
   cache.display();
   std::cout << "Order should be 5<->2<->3\n";
   cache.get("2");
   std::cout << "Getting 2 from cache\n";
   cache.display();
   std::cout << "Order should be 2<->5<->3\n";
   std::cout << "Getting 3 from cache\n";
   cache.get("3");
   std::cout << "Order should be 3<->2<->5\n";
   cache.display();
   std::cout << "Setting 6 to cache\n";
   cache.set("6", 6);
   std::cout << "Order should be 6<->3<->2\n";
   cache.display();
   std::cout << "Setting 7 to cache\n";
   cache.set("7", 7);
   std::cout << "Order should be 7<->6<->3\n";
   cache.display();
   std::cout << "Getting 3 from cache\n";
   cache.get("3");
   std::cout << "Order should be 3<->7<->6\n";
   cache.display();
   std::cout << "Setting 8 to cache\n";
   cache.set("8", 8);
   cache.display();
   std::cout << "Order should be 8<->3<->7\n";
}

/* TODO Change to GTEST
#include <gtest/gtest.h>  // Include the Google Test header
#include <memory>
#include <iostream>
#include <string>
#include <exception>

#define LOG_ERROR std::cout << "[ERROR] : "
#define LOG_WARNING std::cout << "[WARNING] : " 
#define LOG_INFO std::cout << "[INFO] : " 

// Include your existing code here, but remove the main function.
// Ensure that your existing code is encapsulated in a namespace or class, 
// so it doesn't pollute the global namespace.

// Example: Your existing code
// ...

// Define a test fixture class for your tests
class CacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Code to set up your test environment, if needed
    }

    void TearDown() override {
        // Code to clean up your test environment, if needed
    }
};

// Define test cases using the TEST macro
TEST_F(CacheTest, TestCacheFunctionality) {
    // Your test code goes here

    auto cache = Cache<int>(3, "LRU");
    cache.set("1", 1);
    cache.set("2", 2);
    
    // Use Google Test assertions to check the behavior
    EXPECT_EQ(cache.get("1"), 1);
    EXPECT_EQ(cache.get("2"), 2);
    
    // Add more test cases as needed
}

TEST_F(CacheTest, TestCacheEviction) {
    // Your test code for eviction behavior

    auto cache = Cache<int>(2, "LRU");
    cache.set("1", 1);
    cache.set("2", 2);
    cache.set("3", 3);
    
    // Use Google Test assertions to check the eviction behavior
    EXPECT_EQ(cache.get("1"), -1); // Assuming -1 indicates a cache miss
    EXPECT_EQ(cache.get("2"), -1);
    EXPECT_EQ(cache.get("3"), 3);
    
    // Add more test cases as needed
}

// Add more test cases as needed

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

*/