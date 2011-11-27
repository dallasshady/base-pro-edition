/**
 * @file hash.h
 * 
 * templated hash table class.
 * 
 * by Morgan McGuire, matrix@graphics3d.com
 * cite Bug fix by Darius Jazayeri, jazayeri@MIT.EDU
 * created 2001-04-22 @edited  2003-10-06 Copyright 2000-2004, Morgan McGuire.
 *
 * adaptation for d3 by bad3p
 */

#ifndef G3D_TABLE_H
#define G3D_TABLE_H

#include "headers.h"
#include "fundamentals.h"

typedef unsigned __int64 uint64;

class Hashable 
{
public:
    virtual unsigned int hashCode() const = 0;
};

/**
 * int hashing function for use with table.
 */

inline unsigned int hashCode(const int a) 
{
	return a;
}

inline unsigned int hashCode(const uint64 a) 
{
	return unsigned int( a );
}

/**
 * default class pointer hash.
 */

inline unsigned int hashCode(const Hashable* a) 
{
    return a->hashCode();
}

/**
 * default class hash.
 */

inline unsigned int hashCode(const Hashable& a) 
{
    return a.hashCode();
}

/**
 * string hashing function for use with Table.
 */

inline unsigned int hashCode(const std::string& a)
{
	int s = int( a.length() );

	int i = 0;
	unsigned int key = 0;

	while (i < s) 
    {
		key = key ^ ((unsigned int)a[i] << ((i & 3) * 8));
		i++;
	}

	return key;
}

#ifdef _MSC_VER
    // Debug name too long warning
    #pragma warning (disable : 4786)
#endif // _MSC_VER

/**
 * An unordered data structure mapping keys to values.
 * 
 * Key must be a pointer, an int, a std::string, a class with a hashCode() method,
 * or provide overloads for the following <B>two</B> functions: 
 * 
 * <PRE>
 *  unsigned int hashCode(const Key&);
 *  bool operator==(const Key&, const Key&);
 * </PRE>
 * 
 * Periodically check that debugGetLoad() is low (> .1).  When it gets near
 * 1.0 your hash function is bad.
 */

template<class Key, class Value> class Table 
{
public:
    /**
     * the pairs returned by iterator.
     */
    class Entry 
    {
    public:
        Key    key;
        Value  value;
    };
private:
    /**
     * linked list nodes used internally by HashTable.
     */
    class Node 
    {
    public:
        unsigned int      hashCode;
        Entry             entry;
        Node*             next;
    public:
        Node(Key key, Value value, unsigned int hashCode, Node* next) 
        {
          this->entry.key   = key;
          this->entry.value = value;
          this->hashCode    = hashCode;
          this->next        = next;
        }
        /**
        Clones a whole chain;
        */
        Node* clone() 
        {
           return new Node( 
               this->entry.key, 
               this->entry.value, 
               hashCode, 
               (next == NULL) ? NULL : next->clone()
           );
        }
    };
private:
    int     _size; // number of elements in the table.
    Node**  bucket;
    int     numBuckets; // Length of the bucket array.
private:
    /**
     * Re-hashes for a larger bucket size.
     */
    void resize(int numBuckets) 
    {
        Node** oldBucket = bucket;
        bucket = (Node**)calloc( sizeof(Node*), numBuckets );
        for(int b = 0; b < this->numBuckets; b++ ) 
        {
            Node* node = oldBucket[b];         
            while (node != NULL) 
            {
                Node* nextNode = node->next;
        
                // insert at the head of the list
                int i = node->hashCode % numBuckets;
                node->next = bucket[i];
                bucket[i] = node;
        
                node = nextNode;
            }
        }
        free(oldBucket);
        this->numBuckets = numBuckets;
    }
    void copyFrom(const Table<Key, Value>& h) 
    {
        this->_size = h._size;
        this->numBuckets = h.numBuckets;
        this->bucket = (Node**)calloc( sizeof(Node*), numBuckets );
        for( int b = 0; b < this->numBuckets; b++ ) {
            if (h.bucket[b] != NULL) 
            {
                bucket[b] = h.bucket[b]->clone();
            }
        }
    }
    /**
     * frees the heap structures for the nodes.
     */
    void freeMemory() 
    {
        for (int b = 0; b < numBuckets; b++) 
        {
           Node* node = bucket[b];
           while (node != NULL) 
           {
                Node* next = node->next;
                delete node;
                node = next;
           }
        }
        free(bucket);
        bucket     = NULL;
        numBuckets = 0;
        _size      = 0;
    }
public:
    /**
     * creates an empty hash table.  
     * this causes some heap allocation to occur.
     */
    Table() 
    {
        numBuckets = 10;
        _size      = 0;
        bucket     = (Node**)calloc( sizeof(Node*), numBuckets );
    }
    /**
     * Destroys all of the memory allocated by the table, but does <B>not</B>
     * call delete on keys or values if they are pointers.  If you want to
     * deallocate things that the table points at, use getKeys() and Array::deleteAll()
     * to delete them.
     */
    virtual ~Table() 
    {
        freeMemory();
    }
    Table(const Table<Key, Value>& h) 
    {
        this->copyFrom(h);
    }
    Table& operator= (const Table<Key, Value>& h) 
    {
        this->copyFrom(h);
        return *this;
    }
    /**
     * Returns the length of the deepest bucket.
     */
    int debugGetDeepestBucketSize() const 
    {
        int deepest = 0;
        for (int b = 0; b < numBuckets; b++) 
        {
            int     count = 0;
            Node*   node = bucket[b];
            while (node != NULL) {
                node = node->next;
                count ++;
            }
            if (count > deepest) {
                deepest = count;
            }
        }
        return deepest;
    }

    /**
     * A small load (close to zero) means the hash table is acting very
     * efficiently most of the time.  A large load (close to 1) means 
     * the hash table is acting poorly-- all operations will be very slow.
     * A large load will result from a bad hash function that maps too
     * many keys to the same code.
     */
    double debugGetLoad() const 
    {
        return debugGetDeepestBucketSize() / (double)size();
    }
    /**
     * Returns the number of buckets.
     */
    int debugGetNumBuckets() const 
    {
        return numBuckets;
    }
public:
    /**
     * c++ STL style iterator variable.  see begin().
     */
    class Iterator 
    {
    private:
        friend class Table<Key, Value>;
    private:        
        int                index; // bucket index.        
        Node*              node;  // linked list node.
        Table<Key, Value>* table;
        int                numBuckets;
        Node**             bucket;
        bool               isDone;
        /**
         * creates the end iterator.
         */
        Iterator(const Table<Key, Value>* table) : table(const_cast<Table<Key, Value>*>(table)) 
        {
            isDone = true;
        }
        Iterator(const Table<Key, Value>* table, int numBuckets, Node** bucket) :
            table(const_cast<Table<Key, Value>*>(table)),
            numBuckets(numBuckets),
            bucket(bucket) 
        {            
            if (numBuckets == 0) 
            {
                // Empty table
                isDone = true;
                return;
            }
            index = 0;
            node = bucket[index];
            isDone = false;
            findNext();
        }
        /**
         * finds the next element, setting isDone if one can't be found.
         * looks at the current element first.
         */
        void findNext() 
        {
            while (node == NULL) 
            {
                index++;
                if (index >= numBuckets) 
                {
                    isDone = true;
                    break;
                } 
                else 
                {
                    node = bucket[index];
                }
            }
        }
    public:
        inline bool operator!=(const Iterator& other) const 
        {
            return !(*this == other);
        }
        bool operator==(const Iterator& other) const 
        {
            if (other.isDone || isDone) {
                // Common case; check against isDone.
                return (isDone == other.isDone) && (other.table == table);
            } 
            else 
            {
            return (table == other.table) &&
                   (node == other.node) && 
                   (index == other.index);
            }
        }
        /**
         * Pre increment.
         */
        Iterator& operator++() 
        {
            node = node->next;
            findNext();
            return *this;
        }
        /**
         * Post increment (slower than preincrement).
         */
        Iterator operator++(int) 
        {
            Iterator old = *this;
            ++(*this);
            return old;
        }
        const Entry& operator*() const 
        {
            return node->entry;
        }
        Entry* operator->() const {
            return &(node->entry);
        }
        operator Entry*() const {
            return &(node->entry);
        }
    };
    /**
     * C++ STL style iterator method.  Returns the first Entry, which 
     * contains a key and value.  Use preincrement (++entry) to get to
     * the next element.  Do not modify the table while iterating.
     */
    Iterator begin() const 
    {
        return Iterator(this, numBuckets, bucket);
    }
    /**
     * C++ STL style iterator method.  Returns one after the last iterator
     * element.
     */
    const Iterator end() const 
    {
        return Iterator(this);
    }
    /**
     * Removes all elements
     */
    void clear() 
    {
        freeMemory();
        numBuckets = 20;
        _size = 0;
        bucket = (Node**)calloc(sizeof(Node*), numBuckets);
    }  
    /**
     * Returns the number of keys.
     */
    int size() const 
    {
        return _size;
    }
    /**
     * If you insert a pointer into the key or value of a table, you are
     * responsible for deallocating the object eventually.  Inserting 
     * key into a table is O(1), but may cause a potentially slow rehashing.
     */
    void set(const Key& key, const Value& value) 
    {
        unsigned int code = ::hashCode(key);
        unsigned int b = code % numBuckets;
        
        // Go to the bucket
        Node* n = bucket[b];

        // No bucket, so this must be the first
        if (n == NULL) 
        {
            bucket[b] = new Node(key, value, code, NULL);
            ++_size;
            return;
        }

        int bucketLength = 1;

        // Sometimes a bad hash code will cause all elements
        // to collide.  Detect this case and don't rehash when 
        // it occurs; nothing good will come from the rehashing.
        bool allSameCode = true;

        // Try to find the node
        do 
        {
            allSameCode = allSameCode && (code == n->hashCode);
            if ((code == n->hashCode) && (n->entry.key == key)) 
            {
               // Replace the existing node.
               n->entry.value = value;
               return;
            }
            n = n->next;
            ++bucketLength;
        } 
        while (n != NULL);

        const int maxBucketLength = 5;
        if ((bucketLength > maxBucketLength) & ! allSameCode && (numBuckets < _size * 20)) 
        {
            // This bucket was really large; rehash if all elements
            // don't have the same hashcode the number of buckets is reasonable.
            resize(numBuckets * 2 + 1);
        }

        // Not found; insert at the head.
        b = code % numBuckets;
        bucket[b] = new Node(key, value, code, bucket[b]);
        ++_size;
   }
   /**
    * Removes an element from the table if it is present.  It is an error
    * to remove an element that isn't present.
    */
   void remove(const Key& key) 
   {      
      unsigned int code = ::hashCode(key);
      unsigned int b = code % numBuckets;

      // Go to the bucket
      Node* n = bucket[b];

      // Make sure it was found
      debugAssert(n != NULL);

      Node* previous = NULL;

      // Try to find the node
      do {
          if ((code == n->hashCode) && (n->entry.key == key)) {
              // This is the node; remove it
              if (previous == NULL) {
                  bucket[b] = n->next;
              } else {
                  previous->next = n->next;
              }
              // Delete the node
              delete n;
              --_size;
              return;
          }

          previous = n;
          n = n->next;
       } while (n != NULL);

      // Not found! insert at the head.
      debugAssertM(false, "Element not found!");
   }
   /**
    * Returns the value associated with key.
    */
   Value& get(const Key& key) const 
   {
      unsigned int code = ::hashCode(key);
      unsigned int b = code % numBuckets;

      Node* node = bucket[b];

      while (node != NULL) {
         if ((node->hashCode == code) && (node->entry.key == key)) {
            return node->entry.value;
         }
         node = node->next;
      }

      assert( !"Key not found" );
      // The next line is here just to make
      // a compiler warning go away.
      return node->entry.value;
   }
   /**
    * Returns true if key is in the table.
    */
   bool containsKey(const Key &key) const {
       unsigned int code = ::hashCode(key);
       unsigned int b = code % numBuckets;

       Node* node = bucket[b];

       while (node != NULL) {
           if ((node->hashCode == code) && (node->entry.key == key)) {
              return true;
           }
           node = node->next;
       } while (node != NULL);

       return false;
   }
   /**
    * Short syntax for get.
    */
   inline Value& operator[](const Key &key) const {
      return get(key);
   }
   /**
    * Returns an array of all of the keys in the table.
    * You can iterate over the keys to get the values.
    */
   std::vector<Key> getKeys() const 
   {
       std::vector<Key> keyVector;
       getKeys( keyVector );
       return keyVector;
   }
   void getKeys(std::vector<Key>& keyVector) const 
   {
       keyVector.clear();
       for( int i = 0; i < numBuckets; i++ )
       {
           Node* node = bucket[i];
           while (node != NULL) 
           {
               keyVector.push_back( node->entry.key );
               node = node->next;
           }
       }
   }
   /**
    * Calls delete on all of the keys.  Does not clear the table, 
    * however, so you are left with a table of dangling pointers.
    */
   void deleteKeys() 
   {
        std::vector<Key>& keyVector = getKeys();
        // invoke the destructors on the elements
        for( int i = 0; i < keyVector.size(); i++ ) 
        {
           (&keyVector[i])->~Key();
        }
   }
   /**
    Calls delete on all of the values.  This is unsafe--
    do not call unless you know that each value appears
    at most once.

    Does not clear the table, so you are left with a table
    of dangling pointers.
    */
   void deleteValues() 
   {
       for (int i = 0; i < numBuckets; i++) 
       {
           Node* node = bucket[i];
           while (node != NULL) {
               delete node->entry.value;
               node = node->next;
           }
       }
   }
};

#endif
