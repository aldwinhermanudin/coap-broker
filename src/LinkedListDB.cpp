#include <iostream>
#include "LinkedListDB.hpp"

int	string_equal(std::string a, std::string b){
	if (a.empty() || b.empty()) return 0;
	if (a.compare(b) == 0)	return 1;
	else return 0;
}

	auto TopicData::safe_copy(auto &input){	
        data_r_lock();
        auto temp = input;
        data_unlock();
        return temp;
    }
    TopicData::TopicData(const TopicData &input){
        path = input.path;
        data = input.data;
        topic_ma = input.topic_ma;
        data_ma = input.data_ma;
        nextPtr = input.nextPtr;
        pthread_rwlock_init(&(node_lock), NULL);
        pthread_rwlock_init(&(data_lock), NULL);
        // do not copy pthread_rwlock_t        
    }
 
    TopicData::TopicData( std::string path, std::string data, time_t topic_ma, time_t data_ma ){
        this->path = path;
        this->data = data;
        this->topic_ma = topic_ma;
        this->data_ma = data_ma;
        pthread_rwlock_init(&(node_lock), NULL);
        pthread_rwlock_init(&(data_lock), NULL);
    }
    TopicData::TopicData( std::string path, std::string data): TopicData(path, data, 0,0){}
    TopicData::TopicData( std::string path, time_t topic_ma ){
        this->path = path; 
        this->topic_ma = topic_ma;
        pthread_rwlock_init(&(node_lock), NULL);
        pthread_rwlock_init(&(data_lock), NULL);
    }

    int TopicData::node_r_lock(){	
        return pthread_rwlock_rdlock(&node_lock);
    }    
    int TopicData::node_w_lock(){
        return pthread_rwlock_wrlock(&node_lock);
    }    
    int TopicData::node_unlock(){
        return pthread_rwlock_unlock(&node_lock);
    }
    void TopicData::node_lock_destroy(){
        pthread_rwlock_destroy(&node_lock);
    }
    
    int TopicData::data_r_lock(){
        return pthread_rwlock_rdlock(&data_lock);
    }    
    int TopicData::data_w_lock(){
        return pthread_rwlock_wrlock(&data_lock);
    }    
    int TopicData::data_unlock(){
        return pthread_rwlock_unlock(&data_lock);
    }
    void TopicData::data_lock_destroy(){
        pthread_rwlock_destroy(&data_lock);
    }

    std::string& TopicData::get_path(){
        return (path);
    }
    std::string& TopicData::get_data(){
        return (data);
    }
    time_t& TopicData::get_topic_ma(){
        return (topic_ma);
    }
    time_t& TopicData::get_data_ma(){
        return (data_ma);
    }

    std::string TopicData::copy_path(){
        return safe_copy(path);
    }
    std::string TopicData::copy_data(){
        return safe_copy(data);
    }
    time_t TopicData::copy_topic_ma(){
        return safe_copy(topic_ma);
    }
    time_t TopicData::copy_data_ma(){
        return safe_copy(data_ma);
    }

    bool TopicData::update_topic(std::string path, std::string data, time_t topic_ma, time_t data_ma)
    {  
        node_r_lock();
        data_w_lock();
        this->path      = path;
        this->data      = data;
        this->topic_ma  = topic_ma;
        this->data_ma 	= data_ma;
        data_unlock();
        node_unlock();
        return 1;
    }  
    bool TopicData::update_topic(std::string data, time_t data_ma){
        return update_topic(this->path, data, this->topic_ma, data_ma);
    } 
    
    bool TopicData::update_topic(time_t topic_ma){			
        return update_topic(this->path, this->data, topic_ma, this->data_ma);
    }

    int TopicData::delete_topic_data(){ 
        update_topic(this->path, std::string(), this->topic_ma, 0);
    } 

    std::string TopicData::print_topic(){
        node_r_lock();
        data_r_lock();
        std::string temp = path + " " + data + " " + std::to_string(topic_ma) + " " + std::to_string(data_ma);
        data_unlock();
        node_unlock();
        return temp;
	} 

	TopicDB::TopicDB(){}
    TopicDB::TopicDB(TopicData input){

       sPtr = std::make_shared<TopicData>(input);
       this->length_++;
    }

    int TopicDB::get_length(){
        return length_;
    }
    TopicDataPtr TopicDB::get_head(){
        return sPtr;
    }
    TopicDataPtr TopicDB::copy_topic(std::string path){ 
        
        TopicDataPtr previousPtr = nullptr; /* pointer to previous node in list */
        TopicDataPtr currentPtr = nullptr;  /* pointer to current node in list */
        TopicDataPtr tempPtr = nullptr;     /* temporary node pointer */
        
        if (path.empty() || !length_) { 
            return nullptr;
        } 
        else { 
            sPtr->node_r_lock();
        }
        
        if ( string_equal(sPtr->get_path(),path)) { 
           
            tempPtr = std::make_shared<TopicData>(*(sPtr.get()));
            sPtr->node_unlock(); 
            return tempPtr;
        } 
        
        else { 
            previousPtr = sPtr;
            currentPtr = sPtr->nextPtr;
            sPtr->node_unlock();

            while ( currentPtr != nullptr ) {  
                currentPtr->node_r_lock();
                if (string_equal(currentPtr->get_path(),path)){
                    break;
                }
                
                previousPtr = currentPtr;        
                currentPtr = currentPtr->nextPtr;  
                previousPtr->node_unlock();
            } 
 
            if ( currentPtr != nullptr ) { 
                tempPtr = std::make_shared<TopicData>(*(currentPtr.get()));
                currentPtr->node_unlock(); 
                return tempPtr;
            } 
            else{
                return nullptr;
            }
        }  
    }

    TopicDataPtr TopicDB::get_topic(std::string path){ 
        
        TopicDataPtr previousPtr = nullptr; /* pointer to previous node in list */
        TopicDataPtr currentPtr = nullptr;  /* pointer to current node in list */
        TopicDataPtr tempPtr = nullptr;     /* temporary node pointer */
        
        if (path.empty() || !length_) { 
            return nullptr;
        } 
        else { 
            sPtr->node_r_lock(); 
        }
        
        if ( string_equal(sPtr->get_path(),path)) { 
           
            tempPtr = sPtr;
            sPtr->node_unlock(); 
            return tempPtr;
        } 
        
        else { 
            previousPtr = sPtr;
            currentPtr = sPtr->nextPtr;
            sPtr->node_unlock();

            while ( currentPtr != nullptr ) {
                currentPtr->node_r_lock(); 
                if (string_equal(currentPtr->get_path(),path)){
                    break;
                }
                
                previousPtr = currentPtr;        
                currentPtr = currentPtr->nextPtr;  
                previousPtr->node_unlock();
            } 
 
            if ( currentPtr != nullptr ) { 
                tempPtr = currentPtr;
                currentPtr->node_unlock();
                return tempPtr;
            } 
            else{
                return nullptr;
            }
        }  
    } 
    
    int	TopicDB::add_topic(std::string path, time_t topic_ma){
        
        TopicDataPtr newPtr = nullptr;      /* pointer to new node */
        TopicDataPtr previousPtr = nullptr; /* pointer to previous node in list */
        TopicDataPtr currentPtr = nullptr;  /* pointer to current node in list */

            /* add data to new struct here */
            if (!path.empty()){
                newPtr = std::make_shared<TopicData>(path, topic_ma);
            }
            else {
                return 0;
            }

            currentPtr = sPtr;
 
            while ( currentPtr != nullptr && !string_equal(path, currentPtr->get_path())) { 

                currentPtr->node_r_lock();

                previousPtr = currentPtr; 
                currentPtr = currentPtr->nextPtr; 

                previousPtr->node_unlock();
            } 

            if(currentPtr == nullptr){
                if ( previousPtr == nullptr ) { 

                    newPtr->nextPtr = sPtr;
                    sPtr = newPtr;
                } 
                else { 
                    previousPtr->node_w_lock(); 
                    previousPtr->nextPtr = newPtr;
                    newPtr->nextPtr = currentPtr;
                    previousPtr->node_unlock(); 
                } 
                length_++;
                return 1;
            }
            else if (string_equal(path, currentPtr->get_path())){
                currentPtr->update_topic(topic_ma);
                return 1;
            }
            else return 0;
    } 
    
    /* Return 1 if the list is empty, 0 otherwise */
    bool TopicDB::is_empty(){ 
        if(sPtr == nullptr) return true;
        else return false;
    } /* end function isEmpty */

    bool TopicDB::delete_topic(std::string path){ 
        TopicDataPtr previousPtr = nullptr; /* pointer to previous node in list */
        TopicDataPtr currentPtr = nullptr;  /* pointer to current node in list */
        TopicDataPtr tempPtr = nullptr;     /* temporary node pointer */
    
        if (path.empty() || is_empty()) { 
            return false;
        }
        else { 
            sPtr->node_r_lock(); 
        }
        
        /* delete first node */
        if ( string_equal((sPtr)->get_path(),path)) { 
            sPtr->node_unlock();
            sPtr->node_w_lock();

            tempPtr = sPtr; /* hold onto node being removed */
            sPtr = sPtr->nextPtr; /* de-thread the node */
            tempPtr->node_unlock();

            tempPtr->node_lock_destroy();
            tempPtr->data_lock_destroy();
            length_--;
            return true;
        } /* end if */
        else { 
            previousPtr = sPtr;
            currentPtr = ( sPtr )->nextPtr;
            sPtr->node_unlock(); 
             
            while ( currentPtr != nullptr ) { 
                currentPtr->node_r_lock(); 
                if (string_equal(currentPtr->get_path(),path)){
                    break;
                }
                previousPtr = currentPtr; 
                currentPtr = currentPtr->nextPtr; 
                previousPtr->node_unlock();
            }   

            if ( currentPtr != nullptr ) { 
                
                tempPtr = currentPtr;
                currentPtr->node_unlock(); 
                 
                previousPtr->node_w_lock(); 
                currentPtr->node_w_lock();
                
                previousPtr->nextPtr = currentPtr->nextPtr;
                
                currentPtr->node_unlock();
                previousPtr->node_unlock(); 

                tempPtr->node_lock_destroy();
                tempPtr->data_lock_destroy();

                length_--;
                return true;
            } /* end if */
            else{
                return false;
            }
        }  
    } 

    int TopicDB::topic_exist( std::string path )
    { 
        TopicDataPtr topic = copy_topic( path);
        if (topic == nullptr) return 0;
        else return 1;
    } 

    void TopicDB::clean_db(){ 
        
        while(!is_empty()){ 
            TopicDataPtr tempPtr = nullptr;     /* temporary node pointer */
            tempPtr = sPtr; /* hold onto node being removed */
                    
            tempPtr->node_w_lock();
            sPtr = sPtr->nextPtr; /* de-thread the node */
            tempPtr->node_unlock();
                        
            tempPtr->data_lock_destroy();
            tempPtr->node_lock_destroy();
        }
    } 
    std::string TopicDB::print_db(){ 
        TopicDataPtr currentPtr = sPtr;
        std::string temp;
        if ( currentPtr == nullptr ) {
            temp = "List is empty.\n\n";
        } /* end if */
        else { 
            temp = "The list is:\n" ;

            /* while not the end of the list */
            while ( currentPtr != nullptr ) { 

                currentPtr->node_r_lock();
                currentPtr->data_r_lock();

                temp += currentPtr->print_topic() + "--> ";

                TopicDataPtr previousPtr = currentPtr;
                currentPtr = currentPtr->nextPtr;   
                
                previousPtr->data_unlock();
                previousPtr->node_unlock();
            } /* end while */

            temp += "NULL\n\n";
        } /* end else */
        return temp;
    }





 
