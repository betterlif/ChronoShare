#include "event-scheduler.h"

#include <boost/test/unit_test.hpp>
#include <map>
#include <unistd.h>

using namespace boost;
using namespace std;

BOOST_AUTO_TEST_SUITE(SchedulerTests)

map<string, int> table;

void func(string str)
{
  map<string, int>::iterator it = table.find(str);
  if (it == table.end())
  {
    table.insert(make_pair(str, 1));
  }
  else
  {
    int count = it->second;
    count++;
    table.erase(it);
    table.insert(make_pair(str, count));
  }
}

bool
matcher(const TaskPtr &task)
{
  return task->tag() == "period" || task->tag() == "world";
}

BOOST_AUTO_TEST_CASE(SchedulerTest)
{
  SchedulerPtr scheduler(new Scheduler());
  IntervalGeneratorPtr generator(new SimpleIntervalGenerator(0.2));
  IntervalGeneratorPtr randomGen(new RandomIntervalGenerator(0.05, 0.5));

  string tag1 = "hello";
  string tag2 = "world";
  string tag3 = "period";
  string tag4 = "haha";

  TaskPtr task1(new OneTimeTask(boost::bind(func, tag1), tag1, scheduler, 0.5));
  TaskPtr task2(new OneTimeTask(boost::bind(func, tag2), tag2, scheduler, 0.5));
  TaskPtr task3(new PeriodicTask(boost::bind(func, tag3), tag3, scheduler, generator));
  TaskPtr task4(new PeriodicTask(boost::bind(func, tag4), tag4, scheduler, randomGen, 5));

  scheduler->start();
  scheduler->addTask(task1);
  scheduler->addTask(task2);
  scheduler->addTask(task3);
  BOOST_CHECK_EQUAL(scheduler->size(), 3);
  usleep(600000);
  BOOST_CHECK_EQUAL(scheduler->size(), 1);
  scheduler->addTask(task1);
  BOOST_CHECK_EQUAL(scheduler->size(), 2);
  usleep(600000);
  scheduler->addTask(task1);
  BOOST_CHECK_EQUAL(scheduler->size(), 2);
  usleep(400000);
  scheduler->deleteTask(task1->tag());
  BOOST_CHECK_EQUAL(scheduler->size(), 1);
  usleep(200000);

  scheduler->addTask(task1);
  scheduler->addTask(task2);
  BOOST_CHECK_EQUAL(scheduler->size(), 3);
  usleep(100000);
  scheduler->deleteTask(bind(matcher, _1));
  BOOST_CHECK_EQUAL(scheduler->size(), 1);
  usleep(500000);

  scheduler->addTask(task4);
  usleep(500000);

  scheduler->shutdown();

  int hello = 0, world = 0, period = 0, haha = 0;

  map<string, int>::iterator it;
  it = table.find(tag1);
  if (it != table.end())
  {
    hello = it->second;
  }
  it = table.find(tag2);
  if (it != table.end())
  {
    world = it->second;
  }
  it = table.find(tag3);
  if (it != table.end())
  {
    period = it->second;
  }

  it = table.find(tag4);
  if (it != table.end())
  {
    haha = it->second;
  }

  // added four times, canceled once before invoking callback
  BOOST_CHECK_EQUAL(hello, 3);
  // added two times, canceled once by matcher before invoking callback
  BOOST_CHECK_EQUAL(world, 1);
  // invoked every 0.2 seconds before deleted by matcher
  BOOST_CHECK_EQUAL(period, static_cast<int>((0.6 + 0.6 + 0.4 + 0.2 + 0.1) / 0.2));
  // should be invoked 5 times exactly
  BOOST_CHECK_EQUAL(haha, 5);

}

BOOST_AUTO_TEST_CASE(GeneratorTest)
{
  double interval = 10;
  double percent = 0.5;
  int times = 10000;
  IntervalGeneratorPtr generator(new RandomIntervalGenerator(interval, percent));
  double sum = 0.0;
  double min = 2 * interval;
  double max = -1;
  for (int i = 0; i < times; i++)
  {
    double next = generator->nextInterval();
    sum += next;
    if (next > max)
    {
      max = next;
    }
    if (next < min)
    {
      min = next;
    }
  }

  BOOST_CHECK( abs(1.0 - (sum / static_cast<double>(times)) / interval) < 0.05);
  BOOST_CHECK( min > interval * (1 - percent / 2.0));
  BOOST_CHECK( max < interval * (1 + percent / 2.0));
  BOOST_CHECK( abs(1.0 - ((max - min) / interval) / percent) < 0.05);

}

BOOST_AUTO_TEST_SUITE_END()
