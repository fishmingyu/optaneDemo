## OptaneDemo

A simple high-level wrapper of memkind library.

### Basic Background

To better understand the behavior of memkind library, please first check the definitions of modes in optane memory.

* Memory mode
* App direct mode

In Memory Mode, the DRAM acts as a cache for the most frequently-accessed data, while the Optane DC Persistent Memory provides large memory capacity. Cache management operations are handled by the processor's memory controller. When data is requested from memory, the memory controller first accesses the DRAM cache, and if the data is present, the response latency is identical to DRAM. If the data is not in the DRAM cache, it is read from the Optane Memory. This process means that memory mode is transparent to codes and applications. In memory mode, even if we use the memkind library with the pmem setting, the CPU may access the DRAM memory.

In App Direct Mode, applications and operating system would distinguish that there are two types of memory available. When a host is configured in AppDirect, the namespace can be mainly provisioned in one of the following modes: Further information you could visit [website](https://software.intel.com/content/www/us/en/develop/articles/qsg-intro-to-provisioning-pmem.html).

### Environment setup

According to intel's [guideline](https://software.intel.com/content/www/cn/zh/develop/articles/use-memkind-to-manage-volatile-memory-on-intel-optane-persistent-memory.html), before you can use memkind on an Intel Optane DC PMM, the persistent memory must first be provisioned into namespaces to create the logical device **/dev/pmem**. Please find more information about persistent namespaces and how to create them in the [ndctl user guide](https://docs.pmem.io/ndctl-users-guide). 

Note that in memory mode, we couldn't make sure we use optane memory. Thus, we test our envirmoments under settings of appdirect mode.

A basic setup follow could be:

```shell
sudo ipmctl show -memoryresourses #to check the memory status
sudo ipmctl delete -goal #if exists goal
sudo ipmctl create -goal PersistentMemoryType=AppDirect #if no existed goal
reboot
sudo ndctl create-namespace #create namespace, you could check at /dev directory
```

To create a DAX enabled filesystem, you could follow these steps bellow

```bash
sudo mkfs.ext4 /dev/pmem0 (or /dev/pmem1)
sudo mkdir /pmem1
sudo mount -o dax /dev/pmem0 /pmem0
sudo mount -v | grep /pmem0
```

### API intro

```C++
MemoryPool(std::string path, MemoryType mt, int poolSize);
```

The above API could help you construct pmem memory pool easily by assigning DAX file path, memory type and size of memory pool. 
When poolSize is set to zero, allocations are satisfied as long as the temporary file can grow. 
The maximum heap size growth is limited by the capacity of the file system mounted under the path argument.

```C++
Array<int> csrptr(nrow, memorypool);
```

This could create a new array with size of ```nrow * sizeof(int)```. You could access to the array by use its member ```csrptr.array```.

### Results

In appdirect mode, we test serialized spmm with as-caida.mtx.  We test time usage of both writing data to optame memory and SPMM's calculation. 

To test this code, first enter in the main directory

1. run```bash compile.sh```

2. ```shell
   sudo ./optaneSpmm -h #check parser
   ```

You could run with the following example

``` 
sudo ./optaneSpmm -d /pmem0 -m 1 -s ./sparse/as-caida.mtx 
```

#### Switching to Memory Mode

``` shell
umount /pmem0
ndctl list -N #check namespace
ndctl disable-namespace yournamespace
ndctl destory-namespace yournamespace
reboot
```

**Test results on soc-LiveJournal1.mtx under memory mode**
| memory type | Dram  | Optane |
| ----------- | ----- | ------ |
| Time(ms)    | 752.0 | 905.2  |
