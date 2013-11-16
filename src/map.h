#ifndef IMMUTABLE_MAP_H
#define IMMUTABLE_MAP_H 1

#include <cassert>

using std::vector;
using std::shared_ptr;
using std::unique_ptr;
using std::pair;

namespace immutable
{

    template<typename Key,
             typename T,
             typename Hash = std::hash<Key>,
             typename Pred = std::equal_to<Key>,
             typename Alloc = std::allocator<std::pair<const Key,T>>>
    class map {
    public:
      using value_type = std::pair<const Key, T>;
      using key_type = Key;
      using mapped_type = T;
      using hasher = Hash;
      using key_equal = Pred;
      using allocator_type = Alloc;

      class node {
       public:
        // virtual value_type get(size_t hash) = 0; // lookup
        // set with path copying returning updated node copy
        virtual unique_ptr<node> set(size_t hash, value_type value) = 0;
        // unset with path copying returning updated node copy
        // virtual unique_ptr<node> unset(size_t hash) = 0;
      };

      class trie_node : public node {
        const uint32_t presence;
        const vector<shared_ptr<node>> children;

       public:
        // empty node
        trie_node():presence{0}, children{vector<shared_ptr<node>>(0)} {}

        trie_node(uint32_t presence, vector<shared_ptr<node>> children)
          :presence{presence}, children{children} {}

        value_type get(size_t hash);

        unique_ptr<node> set(size_t hash, value_type value) {
          // lookup child index based
          size_t ch_order = child_order(hash);

          // copy children
          vector<shared_ptr<node>> new_children = children;

          if(child_present(hash)) {
            new_children[ch_order] = shared_ptr<node>(children[ch_order]->set(hash >> 5, value));

            assert(new_children.size());

            // return copy of self with the result of set on the child with shifted hash
            return unique_ptr<node>(new trie_node(presence, new_children));
          } else {
            new_children.insert(begin(new_children) + ch_order, std::make_shared<value_node>(value));
            uint32_t new_presence = presence | (1 << (hash & 31));

            trie_node* nn = new trie_node(new_presence, new_children);

            // return copy of self with new value node ptr as the child and updated presence
            return unique_ptr<node>(nn);
          }
        }

        // unique_ptr<node> unset(size_t hash);

        shared_ptr<node> get_child(size_t hash) {
          return children[child_order(hash)];
        }

        size_t child_order(size_t hash) {
          // count one bits to the 'left' from the child position
          return popcnt(presence & ((1 << (hash & 31)) - 1));
        }

        inline bool child_present(size_t hash) {
          return presence & (1 << (hash & 31));
        }

       private:
        static size_t popcnt(size_t i) {
          // TODO check for hardware instruction
          i = i - ((i >> 1) & 0x55555555);
          i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
          return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
        }
      };

      class value_node : public node {
        value_type value;

       public:
        value_node(value_type value):value{value} {}

        value_type get(size_t hash) {
          return value;
        }

        unique_ptr<node> set(size_t hash, value_type v) {
          // return new value_node with v
          // TODO check for the same value and do nothing

          // replace key
          if (value.first == v.first)
            return unique_ptr<node>(new value_node(v));

          // different key with same hash chunk

          // TODO create new trie node with this and new value_node stored
          // based on next hash chunk
        }

        // unique_ptr<node> unset(size_t hash);
      };

      map():root_node{std::make_shared<trie_node>()} {}
      map(shared_ptr<node> root):root_node{root} {}

      // map public interface

      map set(key_type k, mapped_type v) const {
        size_t hash = hasher()(k);
        value_type value = value_type(k, v);

        return map(root_node->set(hash, value));
      }

      mapped_type const &operator[](key_type const &k) const {

      }
     private:
      shared_ptr<node> root_node;

    };
}

#endif

