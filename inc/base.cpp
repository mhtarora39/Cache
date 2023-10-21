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

template <typename Dtype>
struct Buffer
{
public:
   static std::shared_ptr<Buffer<Dtype> > create(const Dtype &buffer)
   {

      return std::shared_ptr<Buffer<Dtype> >(new Buffer<Dtype>(buffer));
   }

   const Dtype &buf() { return m_buffer; }
   void destroy()
   {
      delete this;
   }
   virtual ~Buffer()
   {
   }

protected:
   Buffer(const Dtype &buffer) : m_buffer(buffer),
                                 next(nullptr),
                                 prev(nullptr)
   {
   }

private:
   Dtype m_buffer;
   Buffer<Dtype> *next;
   Buffer<Dtype> *back;
};

template <typename Dtype>
struct EvictionPolicy
{

public:
   virtual std::shared_ptr<Buffer<Dtype>> &get(const std::string &id) = 0;
   virtual void set(std::string &id, std::shared_ptr<Buffer<Dtype>> &data) = 0;
}

template <typename Dtype>
struct LRUEviction : EvictionPolicy<BufDtype>
{

   LRUEviction(size_t cache_limit) : m_cache_limit(cache_limit), first{nullptr}, last{nullptr} {};
   std::shared_ptr<Buffer<Dtype> > &get(const std::string &id)
   {
      // TODO Exception Handling for first access and key not found.
      auto element = m_cache[id];
      if (m_cache.count < 2)
      {
         return element;
      }
      // Updating first points to latest
      // item and insert it at first of the list
      auto next = first->next;
      auto ele_prev = element->prev;
      ele_prev->next = element->next;
      first->next = element;
      ele->next = prev;
      return element;
   }

   void set(std::string &id, std::shared_ptr<Buffer<Dtype>> &data)
   {
      if(m_cache.count == 0) {
         first = data;
      }
      
      m_cache[id] = data;
      // Updating first points to latest
      // item and insert it at first of the list
      auto next = first->next;
      auto ele_prev = data->prev;
      ele_prev->next = data->next;
      first->next = data;
      ele->next = prev;
   }

private:
   size_t m_cache_limit;
   std::unordered_map < std::string, std::shared_ptr < Buffer < Dtype >>>> m_cache;
   std::shared_ptr<Buffer<Dtype> > first;
   std::shared_ptr<Buffer<Dtype> > last;
   int m_count;
};

template <typename BufDtype>
struct Cache
{
   Cache(size_t cache_limit, std::string &eviction_policy)
   {

      // Can be call from factory.But we are supporting only one right now so
      // directly invoking here"

      if (eviction_policy == "LRU")
      {
         m_evic_policy = std::make_shared<LRUEviction<BufDtype> >(new LRUEviction<BufDtype>(5));
      }
      else
      {
         throw Exception("Unsupported Eviction Policy : " + eviction_policy);
      }
   }

   const BufDtype &get(const std::string &id)
   {
      return m_evic_policy->get(id)->buf();
   }

   void set(std::string &id, BufDtype &data)
   {
      auto data_ptr = Buffer<BufDtype>::create(data);
      m_evic_policy->set(id, data);
   }

private:
   std::shared_pre<EvictionPolicy> m_evic_policy;
};
