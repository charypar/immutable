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

      class node;
      using node_ptr = shared_ptr<node>;

      class node {
       public:
        virtual value_type get(size_t hash, key_type key) = 0; // lookup
        // set with path copying returning updated node copy
        virtual unique_ptr<node> set(size_t hash, size_t shift, value_type value, node_ptr this_node) = 0;
        // unset with path copying returning updated node copy
        // virtual unique_ptr<node> unset(size_t hash) = 0;

        size_t child_order(uint32_t presence, size_t truncated_hash) {
          // count one bits to the 'left' from the child position
          return popcnt(presence & ((1 << (truncated_hash & 31)) - 1));
        }

       protected:
        static size_t popcnt(size_t i) {
          // TODO check for hardware instruction
          i = i - ((i >> 1) & 0x55555555);
          i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
          return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
        }
      };


      class trie_node : public node {
        const uint32_t presence;
        const vector<node_ptr> children;

       public:
        // empty node
        trie_node():presence{0}, children{vector<node_ptr>(0)} {}

        trie_node(uint32_t presence, vector<node_ptr> children)
          :presence{presence}, children{children} {}

        value_type get(size_t hash, key_type key) {
          size_t ch_order = child_order(hash);

          if(!child_present(hash))
            throw std::out_of_range("key not found");

          return children[ch_order]->get(hash >> 5, key);
        }

        unique_ptr<node> set(size_t hash, size_t shift, value_type value, node_ptr this_node) {
          // lookup child index based
          size_t ch_order = child_order(hash >> shift);

          // copy children
          vector<shared_ptr<node>> new_children = children;

          if(child_present(hash >> shift)) {
            node_ptr &child = new_children[ch_order];
            child = node_ptr(child->set(hash, shift + 5, value, child));

            // return copy of self with the result of set on the child with shifted hash
            unique_ptr<node> nn(new trie_node(presence, new_children));

            return nn;
          } else {
            // child doesn't yet exist
            new_children.insert(begin(new_children) + ch_order, std::make_shared<value_node>(value));
            uint32_t new_presence = presence | (1 << ((hash >> shift) & 31));

            trie_node* nn = new trie_node(new_presence, new_children);

            // return copy of self with new value node ptr as the child and updated presence
            return unique_ptr<node>(nn);
          }
        }

        // unique_ptr<node> unset(size_t hash);

        inline size_t child_order(size_t truncated_hash) {
          return node::child_order(presence, truncated_hash);
        }

        node_ptr get_child(size_t truncated_hash) {
          return children[child_order(truncated_hash)];
        }

        inline bool child_present(size_t truncated_hash) {
          return presence & (1 << (truncated_hash & 31));
        }

      };

      class value_node : public node {
        value_type value;

       public:
        value_node(value_type value):value{value} {}

        value_type get(size_t hash, key_type key) {
          if(value.first != key)
            throw std::out_of_range("key not found");

          return value;
        }

        unique_ptr<node> set(size_t hash, size_t shift, value_type v, node_ptr this_node) {
          // FIXME check for the same value and do nothing
          if (value.first == v.first) {
            // replace key
            return unique_ptr<node>(new value_node(v));
          } else {
            // create a trie_node containing this value node
            size_t truncated_hash = (hasher()(value.first) >> shift);

            uint32_t new_pres = 1 << (truncated_hash & 31);
            vector<node_ptr> new_children {this_node};

            // FIXME this temporary node will alway get created and then copied and dropped
            // figure out a more efficient way to do this
            node_ptr temp_node = std::make_shared<trie_node>(new_pres, new_children);

            // and recursively call set on it
            return temp_node->set(hash, shift, v, temp_node);
          }
        }

        // unique_ptr<node> unset(size_t hash);
      };

      map():root_node{std::make_shared<trie_node>()} {}
      map(node_ptr root):root_node{root} {}

      // map public interface

      map set(key_type k, mapped_type v) const {
        size_t hash = hasher()(k);
        value_type value = value_type(k, v);

        return map(root_node->set(hash, 0, value, root_node));
      }

      mapped_type at(key_type k) const {
        size_t hash = hasher()(k);
        return root_node->get(hash, k).second;
      }

     private:
      node_ptr root_node;

    };
}

#endif

