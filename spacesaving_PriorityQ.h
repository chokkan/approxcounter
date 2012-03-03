/*
 *      Space Saving algorithm.
 *
 * Copyright (c) 2011 Naoaki Okazaki
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the authors nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//#ifndef __SPACESAVING_H__
//#define __SPACESAVING_H__

#include <unordered_map>
#include <vector>
#include <time.h>

/**
 * Space-saving algorithm.
 *  @param  key_tmpl        Key type.
 *  @param  count_tmpl      Count type.
 */
template <class key_tmpl, class count_tmpl=int>
class spacesaving_PriorityQ
{

public:
    /// Key type.
    typedef key_tmpl key_type;
    /// Count type.
    typedef count_tmpl count_type;
    /// This class.
    typedef spacesaving_PriorityQ<key_tmpl, count_tmpl> this_type;
	
public:
	class item_type
	{
	protected:
		key_type key;       ///< The key
        count_type eps;     ///< Epsilon (maximum overestimation of the count)
        count_type count;   ///< Item count.
		clock_t time;
	
	public:
		
		/**
         * Constructs a count item.
         *  @param  e       The epsilon value.
         */
        item_type()
		: eps(0), count(0)
        {
        }
		
		/**
         * Constructs a count item.
         *  @param  key     The key.
         *  @param  e       The epsilon value.
		 *  @param  c		the count.
         */
        item_type(const key_type& k, count_type e=0, count_type c=0)
		: key(k), eps(e), count(c), time(clock())
        {
        }
		
        /**
         * Gets the key.
         *  @return key_type&   the reference to the key.
         */
        const key_type& get_key() const
        {
            return this->key;
        }
		
        /**
         * Gets the count of the key.
         *  @return count_type  the count.
         */
        count_type get_count() const
        {
            return this->count;
        }
		
        /**
         * Gets the epsilon (maximum overestimation) of the count.
         *  @return count_type  the epsilon value.
         */
        count_type get_epsilon() const
        {
            return this->eps;
        }
		
		/**
         * Gets the time of the key (when inserted)
         *  @return clock_t  the time.
         */
		clock_t get_time() const
		{
			return this->time;
		}
		
		/**
         * Gets the time of the key (when inserted)
         */
		void add_count(count_type freq)
		{
			count+=freq;
			time=clock();
		}
	};

    
protected: //protected param
    /// A mapping type: key -> *item.
    typedef std::unordered_map<key_type, int> keys_t;
    /// The mapping object: key -> *item.
    keys_t m_keys;
    /// The total frequency.
    count_type m_n;
    /// The maximum number of counters.
    count_type m_m;
	/// heap
	std::vector<item_type*> heap;
	

public:
    /**
     * Constructs an object.
     *  @param  m       The maximum number of counters.
     */
    spacesaving_PriorityQ(count_type m=4) : m_n(0), m_m(m)
    {
    }

    /**
     * Destructs the object.
     */
    virtual ~spacesaving_PriorityQ()
    {
    }

protected:
    void addFreq(int& item_id, const count_type& freq)
    {
		heap[item_id]->add_count(freq);
		downheap(item_id);
    }
	
	void push(const key_type& key, const count_type& freq){
		item_type *item = new item_type(key,0,freq);
		heap.push_back(item);
		int item_id = m_keys.size(); 
		m_keys[key] = item_id;
		upheap(item_id);
	}
	
	void popandpush(const key_type& key, const count_type& freq){
	
		key_type frontkey = heap[0]->get_key();
		count_type frontcount = heap[0]->get_count();
		//replace m_keys
		m_keys.erase(frontkey);
		m_keys[key] = 0;
		//replace heap 0
		item_type *item = new item_type(key,frontcount,freq+frontcount);
		heap[0] = item;
		downheap(0);
	}

public:
    void append(const key_type& key, const count_type& freq )
    {
        typename keys_t::iterator it = m_keys.find(key);  
		if (it != m_keys.end()) {
            // add freq to the counter
            this->addFreq(it->second, freq);
			//this->addFreq(m_keys[key],freq);
        } else if ((count_type)m_keys.size() < m_m) {
            // push queue
			this->push(key,freq);
        } else {
            // pop queue.front() and push
			this->popandpush(key,freq);
        }
        ++m_n;
    }
	
	
protected:
	void downheap(int item_id)
	{
		int c = item_id*2+1, sz = heap.size();
		if(c >= sz) return;
		if(c+1 < sz){
			count_type cnt1=heap[c]->get_count(), cnt2=heap[c+1]->get_count();
			if( cnt1 > cnt2) c++;
			if( cnt1 == cnt2 and heap[c]->get_time() > heap[c+1]->get_time()) c++;
		}
		count_type cnt1=heap[item_id]->get_count(), cnt2=heap[c]->get_count();
		if(cnt1 < cnt2) return;
		//if(cnt1 == cnt2 && (heap[item_id]->get_epsilon() > heap[c]->get_epsilon())) return; //sort by epsilon
		if(cnt1 == cnt2 and heap[item_id]->get_time() <= heap[c]->get_time()) { // return; //sort by time
		std::cout << heap[item_id]->get_time() << " , child:" << heap[c]->get_time() <<  std::endl;
		std::cout<<"return"<<std::endl;
		return;
		}
		
		//swap
		key_type s1=heap[item_id]->get_key(), s2=heap[c]->get_key();
		std::swap(m_keys[s1],m_keys[s2]);
		std::swap(heap[item_id],heap[c]);
		downheap(c);
	}
	
	void upheap(int item_id)
	{
		if(item_id==0) return;
		int p=(item_id-1)/2;
		count_type cnt1=heap[item_id]->get_count(), cnt2=heap[p]->get_count();
		if(cnt1 > cnt2) return;
		//if(cnt1 == cnt2 && (heap[item_id]->get_epsilon() < heap[p]->get_epsilon())) return;
		if(cnt1 == cnt2 and heap[item_id]->get_time() >= heap[p]->get_time()) return;
		
		//swap
		key_type s1=heap[item_id]->get_key(), s2=heap[p]->get_key();
		std::swap(m_keys[s1],m_keys[s2]);
		std::swap(heap[item_id],heap[p]);
		upheap(p);
	}

protected:
	item_type pop(){ //for show result
		int sz=heap.size();
		item_type ret = *heap[0];
		heap[0]=heap[sz-1];
		heap.erase(heap.end()-1);
		downheap(0);
		return ret;
	}

public:
	void debug(){
		std::cout<<"****now heap****"<<std::endl;
		for(int i=0;i<heap.size();i++){
			std::cout<<heap[i]->get_key()<<":"<<heap[i]->get_count()<<"    eps:"<<heap[i]->get_epsilon()<<std::endl;
		}
	}
	
	void showresult(){
		std::vector<item_type> result; 
		item_type item;
		while(!heap.empty()){
			item = this->pop();
			result.push_back(item);
		}
		for(int i=result.size()-1;i>0;i--){
			std::cout<<result[i].get_key()<<"\t"<<result[i].get_count()<<"\t"<<result[i].get_epsilon()<<std::endl;
		}
	}
};

//#endif/*__SPACESAVING_H__*/
