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
#include <memory>
#include <iostream>

template <typename Dtype>
struct Buffer
{
public:
   static std::shared_ptr<Buffer<Dtype>> create(Dtype buffer)
   {
      return std::make_shared<Buffer<Dtype>>(buffer);
   }
   const Dtype& buf() { return m_buffer; }

private:
   Dtype m_buffer;
   Buffer(Dtype buffer) : m_buffer(buffer) { std:: cout << "invoked ! \n"; }
   Buffer<Dtype> *next;
   Buffer<Dtype> *back;
};

int main()
{
   auto buffer = Buffer<int>::create(2);
   //std::cout << "Buffer Data : " << buffer->buf() << std::endl;
}

// struct EvictionPolicy {

//    public:
//    virtual BufDtype<BufDtype> &get();
//    virtual set(BufDtype<BufDtype>&);
// }

// template <typename BufDtype>
// struct LRUEviction
// {

//    LRUEviction(size_t cache_limit) : m_cache_limit(cache_limit), first{nullptr}, last{nullptr} {};
//    BufDtype<BufDtype> &get();
//    void set(BufDtype<BufDtype>& data);

// private:
//    size_t m_cache_limit;
//    BufDtype<BufDtype> *first;
//    BufDtype<BufDtype> *last;
// };

// template <typename BufDtype>
// struct Cache
// {
//    Cache(size_t cache_limit, std::string& eviction_policy) {

//       // Can be call from factory.But we are supporting only one right now so  directly invoking
//       // here

//    }

//    private:

// };
