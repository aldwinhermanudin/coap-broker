#ifndef LINKED_LIST_DB_HPP
#define LINKED_LIST_DB_HPP
 
#include <memory>
#include <string>
#include <pthread.h>    
#include <time.h>

int	string_equal(std::string a, std::string b);
class TopicData {  

private:
    std::string path;
	std::string data;		/* each topicData contains a string */
    time_t topic_ma = 0;	// max-age for topic in time after epoch
    time_t data_ma  = 0;		// max-age for data in time after epoch
    pthread_rwlock_t  node_lock;
    pthread_rwlock_t  data_lock;

    auto safe_copy(auto &input);
    
public:
	
    std::shared_ptr<TopicData> nextPtr = nullptr; /* pointer to next node*/ 

    TopicData(const TopicData &input);
    TopicData( std::string path, std::string data, time_t topic_ma, time_t data_ma );
    TopicData( std::string path, std::string data);
    TopicData( std::string path, time_t topic_ma );

    int node_r_lock();
    int node_w_lock();
    int node_unlock();
    void node_lock_destroy();
    int data_r_lock();
    int data_w_lock();
    int data_unlock();
    void data_lock_destroy();
    std::string& get_path();
    std::string& get_data();
    time_t& get_topic_ma();
    time_t& get_data_ma();
    std::string copy_path();
    std::string copy_data();
    time_t copy_topic_ma();
    time_t copy_data_ma();
    int update_topic(std::string path, std::string data, time_t topic_ma, time_t data_ma);
    void update_topic(std::string data, time_t data_ma);
    void update_topic(time_t topic_ma);
    int delete_topic_data();

    std::string print_topic();



}; /* end structure topicData */

using TopicDataPtr = std::shared_ptr<TopicData>;
class TopicDB {            

private:
    int length_ = 0;
    TopicDataPtr sPtr = nullptr;
    
public:
    TopicDB();
    TopicDB(TopicData input);
    int get_length();
    TopicDataPtr get_head();
    TopicDataPtr copy_topic(std::string path);
    TopicDataPtr get_topic(std::string path);
    int	add_topic(std::string path, time_t topic_ma);
    bool is_empty();
    bool delete_topic(std::string path);
    int topic_exist( std::string path );
    void clean_db();
    std::string print_db();
}; 


#endif
