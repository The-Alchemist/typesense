#include <stdlib.h>
#include <iostream>
#include <check.h>
#include <fstream>
#include <chrono>
#include <vector>
#include <time.h>
#include <art.h>
#include "art.h"
#include "topster.h"
#include "forarray.h"
#include "util.h"

using namespace std;

static int test_prefix_cb(void *data, const unsigned char *k, uint32_t k_len, void *val) {
    cout << "#>>>>Key: ";
    printf("%.*s", k_len, k);
    cout << "LENGTH OF IDS: " << ((art_values*)val)->ids.getLength() << endl;

    for(uint32_t i=0; i<((art_values*)val)->ids.getLength(); i++) {
        cout << ", ID: " << ((art_values*)val)->ids.at(i) << endl;
    }
    return 0;
}

void benchmark_heap_array() {
    srand (time(NULL));

    vector<uint32_t> records;

    for(uint32_t i=0; i<10000000; i++) {
        records.push_back((const unsigned int &) rand());
    }

    vector<uint32_t> hits;

    for(uint32_t i=0; i<records.size(); i++) {
        if(i%10 == 0) {
            hits.push_back(i);
        }
    }

    auto begin = std::chrono::high_resolution_clock::now();

    Topster<4000> heapArray;

    for(uint32_t i=0; i<hits.size(); i++) {
        heapArray.add(records[hits[i]]);
    }

    std::sort(std::begin(heapArray.data), std::end(heapArray.data));

    long long int timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin).count();

    for(uint32_t i=0; i<heapArray.size; i++) {
        cout << "Res: " << heapArray.data[i] << endl;
    }

    cout << "Time taken: " << timeMillis << endl;
}

void index_document(art_tree& t, uint32_t doc_id, vector<string> & tokens, uint16_t score) {
    for(auto & token: tokens){
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        art_document document;
        document.id = doc_id;
        document.score = score;
        art_insert(&t, (const unsigned char *) token.c_str(), token.length(), &document);
    }
}

void find_documents(art_tree & t, string q) {
  /*
    1. Split q into tokens
    2. For each token, look up ids using exact lookup
        a. If a token has no result, try again with edit distance of 1, and then 2
    3. Intersect the lists to find docs that match all results
    4. Sort the docs based on some ranking criteria
  */
  vector<string> tokens;
  tokenize(q, tokens, " ", true);

  for(auto token: tokens) {
      int max_cost = 0;
      std::vector<art_leaf*> results;

      do {
          art_iter_fuzzy_prefix(&t, (const unsigned char *) token.c_str(), (int) token.length(), max_cost, results);
          max_cost++;
      } while(results.size() != 0 && max_cost <= 2);

      for(auto leaf: results) {
          for(auto i=0; i<leaf->values->ids.getLength(); i++) {

          }
      }
  }
}

int main() {
    art_tree t;
    art_tree_init(&t);

    std::ifstream infile("/Users/kishorenc/others/wreally/search/test/documents.txt");

    std::string line;
    uint32_t num = 1;

    while (std::getline(infile, line)) {
        vector<string> parts;
        tokenize(line, parts, "\t", true);
        vector<string> tokens;
        tokenize(parts[0], tokens, " ", true);
        index_document(t, num, tokens, stoi(parts[1]));
        num++;
    }

    const unsigned char *prefix = (const unsigned char *) "l";
    size_t prefix_len = strlen((const char *) prefix);
    std::vector<art_leaf*> results;

    auto begin = std::chrono::high_resolution_clock::now();
    art_iter_fuzzy_prefix(&t, prefix, prefix_len, 0, results);
    long long int timeMillis = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();

//    art_iter_prefix(&t, prefix, strlen((const char *) prefix), test_prefix_cb, NULL);
//    art_iter(&t, test_prefix_cb, NULL);

    cout << "Time taken: " << timeMillis << "us" << endl;

    for(auto leaf: results) {
        std::cout << ">>>>/Key: " << leaf->key << " - score: " << leaf->score << std::endl;
        for(uint32_t i=0; i<leaf->values->ids.getLength(); i++) {
            std::cout << ", ID: " << leaf->values->ids.at(i) << std::endl;
        }
        //std::cout << ", Value: " << leaf->values->ids.at(0) << std::endl;
    }

    art_tree_destroy(&t);
    return 0;
}