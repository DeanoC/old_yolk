/*
    Copyright 2005-2011 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks.

    Threading Building Blocks is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    Threading Building Blocks is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Threading Building Blocks; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/

#define TBB_PREVIEW_GRAPH_NODES 1
#include "harness_graph.h"

#include "tbb/task_scheduler_init.h"
#include "tbb/spin_mutex.h"

#if !__SUNPRO_CC

tbb::spin_mutex global_mutex;

#define N 100
#define MAX_NODES 4

//! Performs test on function nodes with limited concurrency and buffering
/** Theses tests check:
    1) that the number of executing copies never exceed the concurreny limit
    2) that the node never rejects
    3) that no items are lost
    and 4) all of this happens even if there are multiple predecessors and successors
*/

template< typename InputType >
struct parallel_put_until_limit : private NoAssign {

    harness_counting_sender<InputType> *my_senders;

    parallel_put_until_limit( harness_counting_sender<InputType> *senders ) : my_senders(senders) {}

    void operator()( int i ) const  {
        if ( my_senders ) {
            my_senders[i].try_put_until_limit();
        }
    }

};
     
//! exercise buffered multioutput_function_node.  
template< typename InputType, typename OutputTuple, typename Body >
void buffered_levels( size_t concurrency, Body body ) {
    typedef typename std::tuple_element<0,OutputTuple>::type OutputType;
    // Do for lc = 1 to concurrency level
    for ( size_t lc = 1; lc <= concurrency; ++lc ) { 
        tbb::flow::graph g;

        // Set the execute_counter back to zero in the harness
        harness_graph_multioutput_executor<InputType, OutputTuple,tbb::spin_mutex>::execute_count = 0;
        // Set the max allowed executors to lc.  There is a check in the functor to make sure this is never exceeded.
        harness_graph_multioutput_executor<InputType, OutputTuple,tbb::spin_mutex>::max_executors = lc;

        // Create the function_node with the appropriate concurreny level, and use default buffering
        tbb::flow::multioutput_function_node< InputType, OutputTuple > exe_node( g, lc, body );
   
        //Create a vector of identical exe_nodes
        std::vector< tbb::flow::multioutput_function_node< InputType, OutputTuple > > exe_vec(2, exe_node);

        // exercise each of the copied nodes
        for (size_t node_idx=0; node_idx<exe_vec.size(); ++node_idx) {
            for (size_t num_receivers = 1; num_receivers <= MAX_NODES; ++num_receivers ) {
                // Create num_receivers counting receivers and connect the exe_vec[node_idx] to them.
                harness_mapped_receiver<OutputType> *receivers = new harness_mapped_receiver<OutputType>[num_receivers];
                for (size_t r = 0; r < num_receivers; ++r ) {
                    tbb::flow::make_edge( tbb::flow::output_port<0>(exe_vec[node_idx]), receivers[r] );
                }

                // Do the test with varying numbers of senders
                harness_counting_sender<InputType> *senders = NULL;
                for (size_t num_senders = 1; num_senders <= MAX_NODES; ++num_senders ) {
                    // Create num_senders senders, set there message limit each to N, and connect them to the exe_vec[node_idx]
                    senders = new harness_counting_sender<InputType>[num_senders];
                    for (size_t s = 0; s < num_senders; ++s ) {
                        senders[s].my_limit = N;
                        tbb::flow::make_edge( senders[s], exe_vec[node_idx] );
                    }

                    // Initialize the receivers so they know how many senders and messages to check for
                    for (size_t r = 0; r < num_receivers; ++r ) {
                         receivers[r].initialize_map( N, num_senders ); 
                    }

                    // Do the test
                    NativeParallelFor( (int)num_senders, parallel_put_until_limit<InputType>(senders) );
                    g.wait_for_all();

                    // cofirm that each sender was requested from N times 
                    for (size_t s = 0; s < num_senders; ++s ) {
                        size_t n = senders[s].my_received;
                        ASSERT( n == N, NULL ); 
                        ASSERT( senders[s].my_receiver == &exe_vec[node_idx], NULL );
                    }
                    // validate the receivers
                    for (size_t r = 0; r < num_receivers; ++r ) {
                        receivers[r].validate();
                    }
                    delete [] senders;
                }
                for (size_t r = 0; r < num_receivers; ++r ) {
                    tbb::flow::remove_edge( tbb::flow::output_port<0>(exe_vec[node_idx]), receivers[r] );
                }
                ASSERT( exe_vec[node_idx].try_put( InputType() ) == true, NULL );
                g.wait_for_all();
                for (size_t r = 0; r < num_receivers; ++r ) {
                    // since it's detached, nothing should have changed
                    receivers[r].validate();
                }
                delete [] receivers;
            }
        } 
    }
}

const size_t Offset = 123;
tbb::atomic<size_t> global_execute_count;

struct inc_functor {

    tbb::atomic<size_t> local_execute_count;
    inc_functor( ) { local_execute_count = 0; }
    inc_functor( const inc_functor &f ) { local_execute_count = f.local_execute_count; }

    template<typename ports_type>
    void operator()( int i, ports_type &p ) {
       ++global_execute_count;
       ++local_execute_count;
       std::get<0>(p).put(i);
    }

};

template< typename InputType, typename OutputTuple >
void buffered_levels_with_copy( size_t concurrency ) {
    typedef typename std::tuple_element<0,OutputTuple>::type OutputType;
    // Do for lc = 1 to concurrency level
    for ( size_t lc = 1; lc <= concurrency; ++lc ) { 
        tbb::flow::graph g;

        inc_functor cf;
        cf.local_execute_count = Offset;
        global_execute_count = Offset;
       
        tbb::flow::multioutput_function_node< InputType, OutputTuple > exe_node( g, lc, cf );

        for (size_t num_receivers = 1; num_receivers <= MAX_NODES; ++num_receivers ) {
           harness_mapped_receiver<OutputType> *receivers = new harness_mapped_receiver<OutputType>[num_receivers];
           for (size_t r = 0; r < num_receivers; ++r ) {
               tbb::flow::make_edge( tbb::flow::output_port<0>(exe_node), receivers[r] );
            }

            harness_counting_sender<InputType> *senders = NULL;
            for (size_t num_senders = 1; num_senders <= MAX_NODES; ++num_senders ) {
                senders = new harness_counting_sender<InputType>[num_senders];
                for (size_t s = 0; s < num_senders; ++s ) {
                    senders[s].my_limit = N;
                    tbb::flow::make_edge( senders[s], exe_node );
                }

                for (size_t r = 0; r < num_receivers; ++r ) {
                    receivers[r].initialize_map( N, num_senders ); 
                }

                NativeParallelFor( (int)num_senders, parallel_put_until_limit<InputType>(senders) );
                g.wait_for_all();

                for (size_t s = 0; s < num_senders; ++s ) {
                    size_t n = senders[s].my_received;
                    ASSERT( n == N, NULL ); 
                    ASSERT( senders[s].my_receiver == &exe_node, NULL );
                }
                for (size_t r = 0; r < num_receivers; ++r ) {
                    receivers[r].validate();
                }
                delete [] senders;
            }
            for (size_t r = 0; r < num_receivers; ++r ) {
                tbb::flow::remove_edge( tbb::flow::output_port<0>(exe_node), receivers[r] );
            }
            ASSERT( exe_node.try_put( InputType() ) == true, NULL );
            g.wait_for_all();
            for (size_t r = 0; r < num_receivers; ++r ) {
                receivers[r].validate();
            }
            delete [] receivers;
        }

        // validate that the local body matches the global execute_count and both are correct
        inc_functor body_copy = tbb::flow::copy_body<inc_functor>( exe_node );
        const size_t expected_count = N/2 * MAX_NODES * MAX_NODES * ( MAX_NODES + 1 ) + MAX_NODES + Offset; 
        size_t global_count = global_execute_count;
        size_t inc_count = body_copy.local_execute_count;
        ASSERT( global_count == expected_count && global_count == inc_count, NULL ); 
    }
}

template< typename InputType, typename OutputTuple >
void run_buffered_levels( int c ) {
    typedef typename tbb::flow::multioutput_function_node<InputType,OutputTuple>::ports_type ports_type;
    harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::max_executors = c;
    #if __TBB_LAMBDAS_PRESENT
    buffered_levels<InputType,OutputTuple>( c, []( InputType i, ports_type &p ) { harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::func(i,p); } );
    #endif
    buffered_levels<InputType,OutputTuple>( c, &harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::func );
    buffered_levels<InputType,OutputTuple>( c, typename harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::functor() );
    buffered_levels_with_copy<InputType,OutputTuple>( c );
}


//! Performs test on executable nodes with limited concurrency
/** Theses tests check:
    1) that the nodes will accepts puts up to the concurrency limit,
    2) the nodes do not exceed the concurrency limit even when run with more threads (this is checked in the harness_graph_executor), 
    3) the nodes will receive puts from multiple successors simultaneously,
    and 4) the nodes will send to multiple predecessors.
    There is no checking of the contents of the messages for corruption.
*/
     
template< typename InputType, typename OutputTuple, typename Body >
void concurrency_levels( size_t concurrency, Body body ) {
    typedef typename std::tuple_element<0,OutputTuple>::type OutputType;
    for ( size_t lc = 1; lc <= concurrency; ++lc ) { 
        tbb::flow::graph g;
        harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::execute_count = 0;

        tbb::flow::multioutput_function_node< InputType, OutputTuple, tbb::flow::rejecting > exe_node( g, lc, body );

        for (size_t num_receivers = 1; num_receivers <= MAX_NODES; ++num_receivers ) {

            harness_counting_receiver<OutputType> *receivers = new harness_counting_receiver<OutputType>[num_receivers];

            for (size_t r = 0; r < num_receivers; ++r ) {
                tbb::flow::make_edge( tbb::flow::output_port<0>(exe_node), receivers[r] );
            }

            harness_counting_sender<InputType> *senders = NULL;
    
            for (size_t num_senders = 1; num_senders <= MAX_NODES; ++num_senders ) {
                {
                    // lock m to prevent exe_node from finishing
                    tbb::spin_mutex::scoped_lock l( harness_graph_multioutput_executor< InputType, OutputTuple, tbb::spin_mutex >::mutex );
    
                    // put to lc level, it will accept and then block at m
                    for ( size_t c = 0 ; c < lc ; ++c ) {
                        ASSERT( exe_node.try_put( InputType() ) == true, NULL );
                    }
                    // it only accepts to lc level
                    ASSERT( exe_node.try_put( InputType() ) == false, NULL );
    
                    senders = new harness_counting_sender<InputType>[num_senders];
                    for (size_t s = 0; s < num_senders; ++s ) {
                       // register a sender
                       senders[s].my_limit = N;
                       exe_node.register_predecessor( senders[s] );
                    }
    
                } // release lock at end of scope, setting the exe node free to continue
                // wait for graph to settle down
                g.wait_for_all();
    
                // confirm that each sender was requested from N times 
                for (size_t s = 0; s < num_senders; ++s ) {
                    size_t n = senders[s].my_received;
                    ASSERT( n == N, NULL ); 
                    ASSERT( senders[s].my_receiver == &exe_node, NULL );
                }
                // confirm that each receivers got N * num_senders + the initial lc puts
                for (size_t r = 0; r < num_receivers; ++r ) {
                    size_t n = receivers[r].my_count;
                    ASSERT( n == num_senders*N+lc, NULL );
                    receivers[r].my_count = 0;
                }
                delete [] senders;
            }
            for (size_t r = 0; r < num_receivers; ++r ) {
                tbb::flow::remove_edge( tbb::flow::output_port<0>(exe_node), receivers[r] );
            }
            ASSERT( exe_node.try_put( InputType() ) == true, NULL );
            g.wait_for_all();
            for (size_t r = 0; r < num_receivers; ++r ) {
                ASSERT( int(receivers[r].my_count) == 0, NULL );
            }
            delete [] receivers;
        }
    }
}

template< typename InputType, typename OutputTuple >
void run_concurrency_levels( int c ) {
    harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::max_executors = c;
    typedef typename tbb::flow::multioutput_function_node<InputType,OutputTuple>::ports_type ports_type;
    #if __TBB_LAMBDAS_PRESENT
    concurrency_levels<InputType,OutputTuple>( c, []( InputType i, ports_type &p ) { harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::func(i,p); } );
    #endif
    concurrency_levels<InputType,OutputTuple>( c, &harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::func );
    concurrency_levels<InputType,OutputTuple>( c, typename harness_graph_multioutput_executor<InputType, OutputTuple, tbb::spin_mutex>::functor() );
}


struct empty_no_assign { 
   empty_no_assign() {}
   empty_no_assign( int ) {}
   operator int() { return 0; }
   operator int() const { return 0; }
};

template< typename InputType >
struct parallel_puts : private NoAssign {

    tbb::flow::receiver< InputType > * const my_exe_node;

    parallel_puts( tbb::flow::receiver< InputType > &exe_node ) : my_exe_node(&exe_node) {}

    void operator()( int ) const  {
        for ( int i = 0; i < N; ++i ) {
            // the nodes will accept all puts
            ASSERT( my_exe_node->try_put( InputType() ) == true, NULL );
        }
    }

};

//! Performs test on executable nodes with unlimited concurrency
/** These tests check:
    1) that the nodes will accept all puts
    2) the nodes will receive puts from multiple predecessors simultaneously,
    and 3) the nodes will send to multiple successors.
    There is no checking of the contents of the messages for corruption.
*/

template< typename InputType, typename OutputTuple, typename Body >
void unlimited_concurrency( Body body ) {
    typedef typename std::tuple_element<0,OutputTuple>::type OutputType;

    for (int p = 1; p < 2*MaxThread; ++p) {
        tbb::flow::graph g;
        tbb::flow::multioutput_function_node< InputType, OutputTuple, tbb::flow::rejecting > exe_node( g, tbb::flow::unlimited, body );

        for (size_t num_receivers = 1; num_receivers <= MAX_NODES; ++num_receivers ) {
            harness_counting_receiver<OutputType> *receivers = new harness_counting_receiver<OutputType>[num_receivers];
            harness_graph_multioutput_executor<InputType, OutputTuple>::execute_count = 0;

            for (size_t r = 0; r < num_receivers; ++r ) {
                tbb::flow::make_edge( tbb::flow::output_port<0>(exe_node), receivers[r] );
            }

            NativeParallelFor( p, parallel_puts<InputType>(exe_node) );
            g.wait_for_all(); 

            // 2) the nodes will receive puts from multiple predecessors simultaneously,
            size_t ec = harness_graph_multioutput_executor<InputType, OutputTuple>::execute_count;
            ASSERT( (int)ec == p*N, NULL ); 
            for (size_t r = 0; r < num_receivers; ++r ) {
                size_t c = receivers[r].my_count;
                // 3) the nodes will send to multiple successors.
                ASSERT( (int)c == p*N, NULL );
            }
        }
    }
}

template< typename InputType, typename OutputTuple >
void run_unlimited_concurrency() {
    typedef typename tbb::flow::multioutput_function_node<InputType,OutputTuple>::ports_type ports_type;
    harness_graph_multioutput_executor<InputType, OutputTuple>::max_executors = 0;
    #if __TBB_LAMBDAS_PRESENT
    unlimited_concurrency<InputType,OutputTuple>( []( InputType i, ports_type &p ) { harness_graph_multioutput_executor<InputType, OutputTuple>::func(i,p); } );
    #endif
    unlimited_concurrency<InputType,OutputTuple>( &harness_graph_multioutput_executor<InputType, OutputTuple>::func );
    unlimited_concurrency<InputType,OutputTuple>( typename harness_graph_multioutput_executor<InputType, OutputTuple>::functor() );
}

template<typename InputType, typename OutputTuple>
struct oddEvenBody {
    typedef typename tbb::flow::multioutput_function_node<InputType,OutputTuple>::ports_type ports_type;
    typedef typename std::tuple_element<0,OutputTuple>::type EvenType;
    typedef typename std::tuple_element<1,OutputTuple>::type OddType;
    void operator() (const InputType &i, ports_type &p) {
        if((int)i % 2) {
            std::get<1>(p).put(OddType(i));
        }
        else {
            std::get<0>(p).put(EvenType(i));
        }
    }
};

template<typename InputType, typename OutputTuple >
void run_multiport_test(int num_threads) {
    typedef typename tbb::flow::multioutput_function_node<InputType, OutputTuple> mo_node_type;
    typedef typename std::tuple_element<0,OutputTuple>::type EvenType;
    typedef typename std::tuple_element<1,OutputTuple>::type OddType;
    tbb::task_scheduler_init init(num_threads);
    tbb::flow::graph g;
    mo_node_type mo_node(g, tbb::flow::unlimited, oddEvenBody<InputType, OutputTuple>() );

    tbb::flow::queue_node<EvenType> q0(g);
    tbb::flow::queue_node<OddType> q1(g);

    tbb::flow::make_edge(tbb::flow::output_port<0>(mo_node), q0);
    tbb::flow::make_edge(tbb::flow::output_port<1>(mo_node), q1);

    for(InputType i = 0; i < N; ++i) {
        mo_node.try_put(i);
    }

    g.wait_for_all();
    for(int i = 0; i < N/2; ++i) {
        EvenType e;
        OddType o;
        ASSERT(q0.try_get(e) && (int)e % 2 == 0, NULL);
        ASSERT(q1.try_get(o) && (int)o % 2 == 1, NULL);
    }
}

//! Tests limited concurrency cases for nodes that accept data messages
void test_concurrency(int num_threads) {
    tbb::task_scheduler_init init(num_threads);
    run_concurrency_levels<int,std::tuple<int> >(num_threads);
    run_concurrency_levels<int,std::tuple<tbb::flow::continue_msg> >(num_threads);
    run_buffered_levels<int, std::tuple<int> >(num_threads);
    run_unlimited_concurrency<int, std::tuple<int> >();
    run_unlimited_concurrency<int,std::tuple<empty_no_assign> >();
    run_unlimited_concurrency<empty_no_assign,std::tuple<int> >();
    run_unlimited_concurrency<empty_no_assign,std::tuple<empty_no_assign> >();
    run_unlimited_concurrency<int,std::tuple<tbb::flow::continue_msg> >();
    run_unlimited_concurrency<empty_no_assign,std::tuple<tbb::flow::continue_msg> >();
    run_multiport_test<int, std::tuple<int, int> >(num_threads);
    run_multiport_test<float, std::tuple<int, double> >(num_threads);
}

int TestMain() { 
    current_executors = 0;
    if( MinThread<1 ) {
        REPORT("number of threads must be positive\n");
        exit(1);
    }
    for( int p=MinThread; p<=MaxThread; ++p ) {
       test_concurrency(p);
   }
   return Harness::Done;
}
#else  // __SUNPRO_CC
int TestMain() { 
   return Harness::Skipped;
}
#endif  // __SUNPRO_CC
