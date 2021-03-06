/*
 * Dummy scheduling class, mapped to range of 5 levels of SCHED_NORMAL policy
 */

#include "sched.h"

/*
 * Timeslice and age threshold are repsented in jiffies. Default timeslice
 * is 100ms. Both parameters can be tuned from /proc/sys/kernel.
 */

#define DUMMY_TIMESLICE		(100 * HZ / 1000)
#define DUMMY_AGE_THRESHOLD	(3 * DUMMY_TIMESLICE)

unsigned int sysctl_sched_dummy_timeslice = DUMMY_TIMESLICE;
static inline unsigned int get_timeslice(void)
{
	return sysctl_sched_dummy_timeslice;
}

unsigned int sysctl_sched_dummy_age_threshold = DUMMY_AGE_THRESHOLD;
static inline unsigned int get_age_threshold(void)
{
	return sysctl_sched_dummy_age_threshold;
}

static inline int has_pushable_tasks(struct rq *rq)
{
	return !plist_head_empty(&rq->rt.pushable_tasks);
}

#ifdef CONFIG_SMP
static void dequeue_pushable_task(struct rq *rq, struct task_struct *p)
{
	plist_del(&p->pushable_tasks, &rq->rt.pushable_tasks);

	/* Update the new highest prio pushable task */
	if (has_pushable_tasks(rq)) {
		p = plist_first_entry(&rq->rt.pushable_tasks,
				      struct task_struct, pushable_tasks);
		rq->rt.highest_prio.next = p->prio;
	} else
		rq->rt.highest_prio.next = MAX_RT_PRIO;
}
#else
static void dequeue_pushable_task(struct rq *rq, struct task_struct *p)
{

}
#endif /* CONFIG_SMP */
/*
 * Init
 */

void init_dummy_rq(struct dummy_rq *dummy_rq, struct rq *rq)
{
	INIT_LIST_HEAD(&dummy_rq->queue);
}

/*
 * Helper functions
 */
static int pull_rt_task(struct rq *this_rq)
{
	TODO
}
static inline struct task_struct *dummy_task_of(struct sched_dummy_entity *dummy_se)
{
	return container_of(dummy_se, struct task_struct, dummy_se);
}

static inline void _enqueue_task_dummy(struct rq *rq, struct task_struct *p)
{
	struct sched_dummy_entity *dummy_se = &p->dummy_se;
	struct list_head *queue = &rq->dummy.queue;
	list_add_tail(&dummy_se->run_list, queue);
}

static inline void _dequeue_task_dummy(struct task_struct *p)
{
	struct sched_dummy_entity *dummy_se = &p->dummy_se;
	list_del_init(&dummy_se->run_list);
}

/*
 * Scheduling class functions to implement
 */

static void enqueue_task_dummy(struct rq *rq, struct task_struct *p, int flags)
{
	_enqueue_task_dummy(rq, p);
	add_nr_running(rq,1);
}

static void dequeue_task_dummy(struct rq *rq, struct task_struct *p, int flags)
{
	_dequeue_task_dummy(p);
	sub_nr_running(rq,1);
}
/*
* Maybe : ?The task at the top of rq has called yield. We can switch it with an other task? 
* See yield_task_rt in rt.c for implementation exemple.
*/
static void yield_task_dummy(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
         dequeue_task_dummy(rq, curr, 1)；
}
/*
 * adding a new task
 * See check_preempt_curr_rt in rt.c for implementation exemple.
 * Comment in rt.c :
 * "Preempt the current task with a newly woken task if needed"
*/
static void check_preempt_curr_dummy(struct rq *rq, struct task_struct *p, int flags)
{
	if (p->prio < rq->curr->prio)
      resched_task(rq->curr); //resched_task is used to mark that a current running task 
}

static struct task_struct *pick_next_task_dummy(struct rq *rq, struct task_struct* prev)
{
	struct dummy_rq *dummy_rq = &rq->dummy;
	struct sched_dummy_entity *next;
	if(!list_empty(&dummy_rq->queue)) {
		next = list_first_entry(&dummy_rq->queue, struct sched_dummy_entity, run_list);
                put_prev_task(rq, prev);
		return dummy_task_of(next);
	} else {
		return NULL;
	}
}
/*
* See put_prev_task_rt in rt.c for implementation exemple.
* Comment in rt.c : ""
*/
static void put_prev_task_dummy(struct rq *rq, struct task_struct *prev)
{
	 if (prev->se.on_rq){
           enqueue_task_dummy(rq, prev,1 );// do not know the flags problem, simply set it to 1
         }
}
/*
* See set_curr_task_rt in rt.c for implementation exemple.
* Comment in rt.c : ""
*/
static void set_curr_task_dummy(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	struct sched_dummy_entity *se = &rq->curr->se;
	p->se.exec_start = rq->clock;
	dequeue_task_dummy(rq,p,1);
}
/*
* The tick function is invoked regularly, every N ms (I think N varies from 1 to 10).
* the function allows you to preempt a task.  Keep in mind that you don’t have to preempt the current 
* task with every tick (it would result in unnecessary overhead). In contrary, you should wait for K ticks, 
* where K is based on the task's priority, for instance.
*
* See task_tick_rt in rt.c for implementation exemple.
* Comment in rt.c : ""
*/
static void task_tick_dummy(struct rq *rq, struct task_struct *curr, int queued) //not finished, do not know queued
{
	update_curr_dummy(rq);
}

/*
* See switched_from_rt in rt.c for implementation exemple.
*
* Comment in rt.c : 
* "When switch from the rt queue, we bring ourselves to a position
* that we might want to pull RT tasks from other runqueues."
*/
static void switched_from_dummy(struct rq *rq, struct task_struct *p)
{
	if (!task_on_rq_queued(p) || rq->rt.rt_nr_running)
		return;

	if (pull_dummy_task(rq))
		resched_curr(rq);
}

/*
* See switched_to_rt in rt.c for implementation exemple.
*
* Comment in rt.c : 
* "When switching a task to RT, we may overload the runqueue
* with RT tasks. In this case we try to push them off to
* other runqueues".
*/
static void switched_to_dummy(struct rq *rq, struct task_struct *p)
{

	if (task_on_rq_queued(p) && rq->curr != p) {
#ifdef CONFIG_SMP
		//TODO ???
#endif /* CONFIG_SMP */
		if (p->prio < rq->curr->prio)
			resched_curr(rq);
	}
}

/*
* See prio_changed_rt in rt.c for implementation exemple.
*
* Comment in rt.c : 
* "Priority of the task has changed. This may cause
* us to initiate a push or pull."
*/
static void prio_changed_dummy(struct rq*rq, struct task_struct *p, int oldprio)
{
	if (!task_on_rq_queued(p))
		return;

	if (rq->curr == p) {

#ifdef CONFIG_SMP
		//TODO???
#endif /* CONFIG_SMP */

	} 
	else 
	{
	
		if (p->prio < rq->curr->prio)
			resched_curr(rq);
	}
}

/*
* Seeget_rr_interval_rt in rt.c for implementation exemple.
*
* Comment in rt.c : ""
*/
static unsigned int get_rr_interval_dummy(struct rq* rq, struct task_struct *p)
{
	return get_timeslice();
}
#ifdef CONFIG_SMP
/*
 * SMP related functions	
 */

static inline int select_task_rq_dummy(struct task_struct *p, int cpu, int sd_flags, int wake_flags)
{
	int new_cpu = smp_processor_id();
	
	return new_cpu; //set assigned CPU to zero
}


static void set_cpus_allowed_dummy(struct task_struct *p,  const struct cpumask *new_mask)
{
	struct rq *rq;
	int weight;

	BUG_ON(!rt_task(p));

	if (!task_on_rq_queued(p))
		return;

	weight = cpumask_weight(new_mask);

	/*
	 * Only update if the process changes its state from whether it
	 * can migrate or not.
	 */
	if ((p->nr_cpus_allowed > 1) == (weight > 1))
		return;
	rq = task_rq(p);
}
#endif
/*
 * Scheduling class
 */
static void update_curr_dummy(struct rq*rq)
{
}
const struct sched_class dummy_sched_class = {
	.next			= &idle_sched_class,
	.enqueue_task		= enqueue_task_dummy,
	.dequeue_task		= dequeue_task_dummy,
	.yield_task		= yield_task_dummy,

	.check_preempt_curr	= check_preempt_curr_dummy,
	
	.pick_next_task		= pick_next_task_dummy,
	.put_prev_task		= put_prev_task_dummy,

struct dummy_rq {
	struct list_head queue;
};

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_dummy,
	.set_cpus_allowed	= set_cpus_allowed_dummy,
#endif

	.set_curr_task		= set_curr_task_dummy,
	.task_tick		= task_tick_dummy,

	.switched_from		= switched_from_dummy,
	.switched_to		= switched_to_dummy,
	.prio_changed		= prio_changed_dummy,

	.get_rr_interval	= get_rr_interval_dummy,
	.update_curr		= update_curr_dummy,
};

/* LINKS
* about methodes we can use : http://www.makelinux.net/books/lkd2/ch04lev1sec2
*/
