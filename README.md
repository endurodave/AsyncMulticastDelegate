# Asynchronous Multicast Delegates in C++
A C++ standards compliant delegate library capable of targeting any callable function synchronously or asynchronously.

Originally published on CodeProject at: <a href="http://www.codeproject.com/Articles/1160934/Asynchronous-Multicast-Delegates-in-Cplusplus"><strong>Asynchronous Multicast Delegates in C++</strong></a>

<h2>Introduction</h2>

<p>Nothing seems to garner the interest of C++ programmers more than delegates. In other languages, the delegate is a first-class feature so developers can use these well-understood constructs. In C++, however, a delegate is not natively available. Yet that doesn&rsquo;t stop us programmers from trying to emulate the ease with which a delegate stores and invokes any callable function.</p>

<p>Delegates normally support synchronous executions, that is, when invoked, the bound function is executed within the caller&rsquo;s thread of control. On multi-threaded applications, it would be ideal to specify the target function and the thread it should execute on without imposing function signature limitations. The library does the grunt work of getting the delegate and all argument data onto the destination thread. The idea behind this article is to provide a C++ delegate library with a consistent API that is capable of synchronous and asynchronous invocations on any callable function.</p>

<p>The features of the delegate library are:</p>

<ol>
	<li><strong>Any Compiler</strong> &ndash; standard C++ code for any compiler without weird hacks</li>
	<li><strong>Any Function</strong> &ndash; invoke any callable function: member, static, or free</li>
	<li><strong>Any Argument Type</strong> &ndash; supports any argument type: value, reference, pointer, pointer to pointer</li>
	<li><strong>Multiple Arguments</strong> &ndash; supports multiple function arguments</li>
	<li><strong>Synchronous Invocation</strong> &ndash; call the bound function synchronously</li>
	<li><strong>Asynchronous Invocation</strong> &ndash; call the bound function asynchronously on a client specified thread</li>
	<li><strong>Blocking Asynchronous Invocation</strong> - invoke asynchronously using blocking or non-blocking delegates</li>
	<li><strong>Smart Pointer Support</strong> - bind an instance function using a raw object pointer or <code>std::shared_ptr</code></li>
	<li><strong>Automatic Heap Handling</strong> &ndash; automatically copy argument data to the heap for safe transport through a message queue</li>
	<li><strong>Fixed Block Allocator</strong> &ndash; optionally divert heap allocation to fixed block memory pools</li>
	<li><strong>Any OS</strong> &ndash; easy porting to any OS. Win32 and <code>std::thread</code> ports included</li>
	<li><strong>Visual Studio and Eclipse</strong> - VC++ and GCC projects included</li>
	<li><strong>Unit Tests</strong> - extensive unit testing of the delegate library included</li>
	<li><strong>No External Libraries</strong> &ndash; delegate does not rely upon external libraries</li>
	<li><strong>Ease of Use</strong> &ndash; match the <code>FastDelegate</code> API as close as possible</li>
</ol>

<p>The delegate implementation significantly eases multithreaded application development by executing the delegate function with all of the function arguments on the thread of control that you specify. The framework handles all of the low-level machinery to safely invoke any function signature on a target thread.</p>

<p>Windows 2008, 2015 and Eclipse projects are included for easy experimentation. While the Windows operating system provides threads, locks and message queues, the code is partitioned for easy porting to other embedded or PC-based systems. Building the <code>std::thread</code> version means any C++11 compiler supporting the C++ Standard Library thread API is able to use the delegates with no porting effort.</p>

<p>Three asynchronous multicast callback implementations are available: Two in C++ and one written in C. See the <a href="#References">References</a> section for the two other related articles.</p>

<p>A remote procedure call (RPC) using C++ delegates extends this&nbsp;library to include inter-process and inter-processor communications. See the <a href="#References">References</a> section for the this related article.</p>

<p>I&rsquo;ve created four versions of the &ldquo;asynchronous callback&rdquo; idea; three C++ versions and one C version. See the&nbsp;<strong>References</strong>&nbsp;section at the end of the article for links to the other implementations.</p>

<h2>Delegates Background</h2>

<p>If you&rsquo;re not familiar with a delegate, the concept is quite simple. A delegate can be thought of as a super function pointer. In C++, there&#39;s no pointer type capable of pointing to all the possible function variations: instance member, virtual, const, static, and free (global). A function pointer can&rsquo;t point to instance member functions, and pointers to member functions have all sorts of limitations. However, delegate classes can, in a type-safe way, point to any function provided the function signature matches. In short, a delegate points to any function with a matching signature to support anonymous function invocation.</p>

<p>Probably the most famous C++ delegate implementation is the <code>FastDelegate</code> by Doug Clugston. I&rsquo;ve used this code with great success on many different projects. It&rsquo;s easy to use and I&rsquo;ve yet to find a compiler that it doesn&#39;t work on.</p>

<p>While the usage of <code>FastDelegate</code> is seamless (and fast!), examination of the code reveals numerous hacks and &ldquo;horrible&rdquo; casts to make it work universally across different compilers. When I first studied the source, I almost didn&rsquo;t use it on a project just on looks alone because of the complexity. However, it got me to thinking; say I didn&rsquo;t care so much about speed. Was it even possible to design a C++ standards compliant delegate? If so, could it match the interface and usability of <code>FastDelegate</code>?</p>

<p>In practice, while a delegate is useful, a multicast version significantly expands its utility. The ability to bind more than one function pointer and sequentially invoke all registrars&rsquo; makes for an effective publisher/subscriber mechanism. Publisher code exposes a delegate container and one or more anonymous subscribers register with the publisher for callback notifications.</p>

<p>The problem with callbacks on a multithreaded system, whether it be a delegate-based or function pointer based, is that the callback occurs synchronously. Care must be taken that a callback from another thread of control is not invoked on code that isn&rsquo;t thread-safe. Multithreaded application development is hard. It&#39;s hard for the original designer; it&#39;s hard because engineers of various skill levels must maintain the code; it&#39;s hard because bugs manifest themselves in difficult ways. Ideally, an architectural solution helps to minimize errors and eases application development.</p>

<p>Some may question why <code>std::function</code> wasn&rsquo;t used as the basis for an asynchronous delegate. Originally, I started implementing a version using <code>std::function</code> for targeting any callable function and it worked except for one key feature: equality. I soon discovered that you can&rsquo;t compare <code>std::function</code> for equality which is necessary to unregister from the container. There seemed to be no easy, generic way around this. And without a means to remove previously added callable functions, the design was kaput. All is not lost. The delegate hierarchy I ultimately created actually ended up being an advantage for the feature set I was trying to accomplish. Plus it was fun to create.</p>

<p>The article I wrote here on Code Project entitled &ldquo;<a href="https://www.codeproject.com/Articles/1092727/Asynchronous-Multicast-Callbacks-with-Inter-Thread"><strong>Asynchronous Multicast Callbacks with Inter-Thread Messaging</strong></a>&rdquo; provided an asynchronous multicast callback similar in concept to what is proposed here, but the callback signature was fixed and only one templatized function argument was supported. It also limited the callback function type to static member or free functions. Instance member functions were not supported. The advantage of accepting these limitations is that the <code>AsycnCallback&lt;&gt;</code> implementation is much simpler and compact.</p>

<p>This C++ delegate implementation is full featured and allows calling any function, even instance member functions, with any arguments either synchronously or asynchronously. The delegate library makes binding to and invoking any function a snap.</p>

<h2>Using the Code</h2>

<p>I&rsquo;ll first present how to use the code, and then get into the implementation details.</p>

<p>The delegate library is comprised of delegates and delegate containers. A delegate is capable of binding to a single callable function. A multicast delegate container holds one or more delegates in a list to be invoked sequentially. A single cast delegate container holds at most one delegate.</p>

<p>The primary delegate classes are listed below, where <code>X</code> is the number of arguments in the target function signature. For instance, if the target signature uses one argument such as in <code>void (int)</code>, then the <code>DelegateFree1&lt;&gt;</code> version is used. Similarly, if three arguments are used as in <code>void (int, float, char)</code>, the <code>DelgateFree3&lt;&gt;</code> is utilized.</p>

<ul class="class">
	<li><code>DelegateFreeX&lt;&gt;</code></li>
	<li><code>DelegateFreeAsyncX&lt;&gt;</code></li>
	<li><code>DelegateFreeAsyncWaitX&lt;&gt;</code></li>
	<li><code>DelegateMemberX&lt;&gt;</code></li>
	<li><code>DelegateMemberAsyncX&lt;&gt;</code></li>
	<li><code>DelegateMemberAsyncWaitX&lt;&gt;</code></li>
	<li><code>DelegateMemberSpX&lt;&gt;</code></li>
	<li><code>DelegateMemberSpAsyncX&lt;&gt;</code></li>
	<li><code>DelegateRemoteSendX&lt;&gt;</code></li>
	<li><code>DelegateFreeRemoteRecvX&lt;&gt;</code></li>
	<li><code>DelegateMemberRemoteRecvX&lt;&gt;</code></li>
</ul>

<p><code>DelegateFreeX&lt;&gt;</code> binds to a free or static member function. <code>DelegateMemberX&lt;&gt;</code> binds to a class instance member function. <code>DelegateMemberSpX&lt;&gt;</code> binds to a class instance member function using a <code>std::shared_ptr</code> instead of a raw object pointer. All versions offer synchronous function invocation.</p>

<p><code>DelegateFreeAsyncX&lt;&gt;</code><font color="#111111" face="Segoe UI, Arial, sans-serif"><span style="font-size: 14px">, </span></font><code>DelegateMemberAsyncX&lt;&gt;</code> and <code>DelegateMemberSpAsyncX&lt;&gt;</code> operate in the same way as their synchronous counterparts; except these versions offer non-blocking asynchronous function execution on a specified thread of control.</p>

<p><code>DelegateFreeAsyncWaitX&lt;&gt;</code> and <code>DelegateMemberAsyncWaitX&lt;&gt;</code> provides blocking asynchronous function execution on a target thread with a caller supplied maximum wait timeout.</p>

<p><code>DelegateRemoteSendX&lt;&gt;</code>, <code>DelegateFreeRemoteRecvX&lt;&gt;</code> and <code>DelegateMemberRemoteRecvX&lt;&gt;</code> are explained in the article <a href="https://www.codeproject.com/Articles/5262271/Remote-Procedure-Calls-using-Cplusplus-Delegates">Remote Procedure Calls using C++ Delegates</a>.&nbsp;</p>

<p>The three main delegate container classes are:</p>

<ul class="class">
	<li><code>SinglecastDelegateX&lt;&gt;</code></li>
	<li><code>MulticastDelegateX&lt;&gt;</code></li>
	<li><code>MulticastDelegateSafeX&lt;&gt;</code></li>
</ul>

<p><code>SinglecastDelegateX&lt;&gt;</code> is a delegate container accepting a single delegate. The advantage of the single cast version is that it is slightly smaller and allows a return type other than <code>void</code> in the bound function.</p>

<p><code>MulticastDelegateX&lt;&gt;</code> is a delegate container implemented as a singly-linked list accepting multiple delegates. Only a delegate bound to a function with a <code>void</code> return type may be added to a multicast delegate container.</p>

<p><code>MultcastDelegateSafeX&lt;&gt;</code> is a thread-safe container implemented as a singly-linked list accepting multiple delegates. Always use the thread-safe version if multiple threads access the container instance.</p>

<p>Each container stores the delegate by value. This means the delegate is copied internally into either heap or fixed block memory depending on the mode. The user is not required to manually create a delegate on the heap before insertion into the container. Typically, the overloaded template function <code>MakeDelegate()</code> is used to create a delegate instance based upon the function arguments.</p>

<h3>Synchronous Delegates</h3>

<p>All delegates are created with the overloaded <code>MakeDelegate()</code> template function. The compiler uses template argument deduction to select the correct <code>MakeDelegate()</code> version eliminating the need to manually specify the template arguments. For example, here is a simple free function.</p>

<pre lang="C++">
void FreeFuncInt(int value)
{
      cout &lt;&lt; &quot;FreeCallback &quot; &lt;&lt; value &lt;&lt; endl;
}</pre>

<p>To bind the free function to a delegate, create a <code>DelegateFree1&lt;int&gt;</code> instance using <code>MakeDelegate()</code>. The <code>DelegateFree</code> template argument is the <code>int</code> function parameter. <code>MakeDelegate()</code> returns a <code>DelegateFree1&lt;int&gt;</code> object and the following line invokes the function <code>FreeFuncInt</code> using the delegate.</p>

<pre lang="C++">
// Create a delegate bound to a free function then invoke
DelegateFree1&lt;int&gt; delegateFree = MakeDelegate(&amp;FreeFuncInt);
delegateFree(123);</pre>

<p>A member function is bound to a delegate in the same way, only this time <code>MakeDelegate()</code> uses two arguments: a class instance and a member function pointer. The two <code>DelegateMember1</code> template arguments are the class name and the function argument.</p>

<pre lang="C++">
// Create a delegate bound to a member function then invoke
DelegateMember1&lt;TestClass, TestStruct*&gt; delegateMember = 
                MakeDelegate(&amp;testClass, &amp;TestClass::MemberFunc);
delegateMember(&amp;testStruct);</pre>

<p>Rather than create a concrete free or member delegate, typically a delegate container is used to hold one or more delegates. A delegate container can hold any delegate type. For example, a multicast delegate container that binds to any function with a <code>void (int)</code> function signature is shown below.</p>

<pre lang="C++">
MulticastDelegate1&lt;int&gt; delegateA;</pre>

<p>A single cast delegate is created in the same way.</p>

<pre lang="C++">
SinglecastDelegate1&lt;int&gt; delegateB;
</pre>

<p>A function signature that returns a value, such as <code>float (int)</code>, is defined by adding an additional template argument.</p>

<pre lang="C++">
SinglecastDelegate1&lt;int, float&gt; delegateC;</pre>

<p>A <code>SinglecastDelegate&lt;&gt;</code> may bind to a function that returns a value whereas a multicast versions cannot. The reason is that when multiple callbacks are invoked, which callback function return value should be used? The correct answer is none, so multicast containers only accept delegates with function signatures using <code>void</code> as the return type.</p>

<p>More function arguments mean using the <code>MulticastDelegate2</code> or <code>MulticastDelegate3</code> versions. Currently, the library supports up to five function arguments.</p>

<pre lang="C++">
MulticastDelegate2&lt;int, int&gt; delegateD;

MulticastDelegate3&lt;float, int, char&gt; delegateE;</pre>

<p>Of course, more than just built-in pass by value argument types are supported.</p>

<pre lang="C++">
MulticastDelegate3&lt;const MyClass&amp;, MyStruct*, Data**&gt; delegateF;</pre>

<p>Creating a delegate instance and adding it to the multicast delegate container is accomplished with the overloaded <code>MakeDelegate()</code> function and <code>operator+=</code>. Binding a free function or static function only requires a single function pointer argument.</p>

<pre lang="C++">
delegateA += MakeDelegate(&amp;FreeFuncInt);</pre>

<p>An instance member function can also be added to any delegate container. For member functions, the first argument to <code>MakeDelegate()</code> is a pointer to the class instance. The second argument is a pointer to the member function.</p>

<pre lang="C++">
delegateA += MakeDelegate(&amp;testClass, &amp;TestClass::MemberFunc);</pre>

<p>Check for registered clients first, then invoke callbacks for all registered delegates. If multiple delegates are stored within <code>MulticastDelegate1&lt;int&gt;</code>, each one is called sequentially.</p>

<pre>
// Invoke the delegate target functions
if (delegateA)
      delegateA(123);</pre>

<p>Removing a delegate instance from the delegate container uses <code>operator-=</code>.</p>

<pre>
delegateA -= MakeDelegate(&amp;FreeFuncInt);</pre>

<p>Alternatively, <code>Clear()</code> is used to remove all delegates within the container.</p>

<pre>
delegateA.Clear();</pre>

<p>A delegate is added to the single cast container using <code>operator=</code>.</p>

<pre lang="C++">
SinglecastDelegate1&lt;int, int&gt; delegateF;
delegateF = MakeDelegate(&amp;FreeFuncIntRetInt);</pre>

<p>Removal is with <code>Clear()</code> or assigning <code>0</code>.</p>

<pre lang="C++">
delegateF.Clear();
delegateF = 0;</pre>

<h3>Asynchronous Non-Blocking Delegates</h3>

<p>Up until this point, the delegates have all been synchronous. The asynchronous features are layered on top of the synchronous delegate implementation. To use asynchronous delegates, a thread-safe delegate container safely accessible by multiple threads is required. Locks protect the class API against simultaneous access. The &ldquo;Safe&rdquo; version is shown below.</p>

<pre lang="C++">
MulticastDelegateSafe1&lt;TestStruct*&gt; delegateC;</pre>

<p>A thread pointer as the last argument to <code>MakeDelegate()</code> forces creation of an asynchronous delegate. In this case, adding a thread argument causes <code>MakeDelegate()</code> to return a <code>DelegateMemberAsync1&lt;&gt;</code> as opposed to <code>DelegateMember1&lt;&gt;</code>.</p>

<pre lang="C++">
delegateC += MakeDelegate(&amp;testClass, &amp;TestClass::MemberFunc, &amp;workerThread1);</pre>

<p>Invocation is the same as the synchronous version, yet this time the callback function <code>TestClass::MemberFunc()</code> is called from <code>workerThread1</code>.</p>

<pre lang="C++">
if (delegateC)
      delegateC(&amp;testStruct);</pre>

<p>Here is another example of an asynchronous delegate being invoked on <code>workerThread1</code> with <code>std::string</code> and <code>int</code> arguments.</p>

<pre lang="C++">
// Create delegate with std::string and int arguments then asychronously
// invoke on a member function
MulticastDelegateSafe2&lt;const std::string&amp;, int&gt; delegateH;
delegateH += MakeDelegate(&amp;testClass, &amp;TestClass::MemberFuncStdString, &amp;workerThread1);
delegateH(&quot;Hello world&quot;, 2016);</pre>

<p>Usage of the library is consistent between synchronous and asynchronous delegates. The only difference is the addition of a thread pointer argument to<code> MakeDelegate()</code>. Remember to always use the thread-safe <code>MulticastDelegateSafeX&lt;&gt;</code> containers when using asynchronous delegates to callback across thread boundaries.</p>

<p>The default behavior of the delegate library when invoking non-blocking asynchronous delegates is that arguments not passed by value are copied into heap memory for safe transport to the destination thread. This means all arguments will be duplicated. If your data is something other than plain old data (POD) and can&rsquo;t be bitwise copied, then be sure to implement an appropriate copy constructor to handle the copying yourself.</p>

<p>Actually there is a way to defeat the copying and really pass a pointer without copying what it&rsquo;s pointing at. However, the developer must ensure that (a) the pointed to data still exists when the target thread invokes the bound function and (b) the pointed to object is thread safe. This technique is described later in the article.</p>

<p>For more examples, see <em>main.cpp</em> and <em>DelegateUnitTests.cpp</em> within the attached source code.</p>

<h4>Bind to std::shared_ptr</h4>

<p>Binding to instance member function requires a pointer to an object. The delegate library supports binding with a raw pointer and a <code>std::shared_ptr</code> smart pointer. Usage is what you&rsquo;d expect; just use a <code>std::shared_ptr</code> in place of the raw object pointer in the call to <code>MakeDelegate()</code>. Depending on if a thread argument is passed to <code>MakeDelegate()</code> or not, a <code>DelegateMemberSpX&lt;&gt;</code> or <code>DelegateMemberSpAsyncX&lt;&gt;</code> instance is returned.</p>

<pre lang="C++">
// Create a shared_ptr, create a delegate, then synchronously invoke delegate function
std::shared_ptr&lt;TestClass&gt; spObject(new TestClass());
auto delegateMemberSp = MakeDelegate(spObject, &amp;TestClass::MemberFuncStdString);
delegateMemberSp(&quot;Hello world using shared_ptr&quot;, 2016);</pre>

<p>The included VC2008 can&rsquo;t use <code>std::shared_ptr</code> because the compiler doesn&rsquo;t support the feature. Run the VS2015 project for working examples using <code>std::shared_ptr</code>.</p>

<h4>Caution Using Raw Object Pointers</h4>

<p>Certain asynchronous delegate usage patterns can cause a callback invocation to occur on a deleted object. The problem is this: an object function is bound to a delegate and invoked asynchronously, but before the invocation occurs on the target thread, the target object is deleted. In other words, it is possible for an object bound to a delegate to be deleted before the target thread message queue has had a chance to invoke the callback. The following code exposes the issue.</p>

<pre lang="C++">
// Example of a bug where the testClassHeap is deleted before the asychronous delegate 
// is invoked on the workerThread1. In other words, by the time workerThread1 calls
// the bound delegate function the testClassHeap instance is deleted and no longer valid.
TestClass* testClassHeap = new TestClass();
auto delegateMemberAsync = 
    MakeDelegate(testClassHeap, &amp;TestClass::MemberFuncStdString, &amp;workerThread1);
delegateMemberAsync(&quot;Function async invoked on deleted object. Bug!&quot;, 2016);
delegateMemberAsync.Clear();
delete testClassHeap;
</pre>

<p>The example above is contrived, but it does clearly show that nothing prevents an object being deleted while waiting for the asynchronous invocation to occur. In many embedded system architectures, the registrations might occur on singleton objects or objects that have a lifetime that spans the entire execution. In this way, the application&rsquo;s usage pattern prevents callbacks into deleted objects. However, if objects pop into existence, temporarily subscribe to a delegate for callbacks, then get deleted later the possibility of a latent delegate stuck in a message queue could invoke a function on a deleted object.</p>

<p>Fortunately, C++ smart pointers are just the ticket to solve these complex object lifetime issues. A <code>DelegateMemberSpAsyncX&lt;&gt;</code> delegate binds using a <code>std::shared_ptr</code> instead of a raw object pointer. Now that the delegate has a shared pointer, the danger of the object being prematurely deleted is eliminated. The shared pointer will only delete the object pointed to once all references are no longer in use. In the code snippet below, all references to <code>testClassSp</code> are removed by the client code yet the delegate&rsquo;s copy placed into the queue prevents <code>TestClass</code> deletion until after the asynchronous delegate callback occurs.</p>

<pre lang="C++">
// Example of the smart pointer function version of the delegate. The testClassSp instance 
// is only deleted after workerThread1 invokes the callback function thus solving the bug.
std::shared_ptr&lt;TestClass&gt; testClassSp(new TestClass());
auto delegateMemberSpAsync = 
    MakeDelegate(testClassSp, &amp;TestClass::MemberFuncStdString, &amp;workerThread1);
delegateMemberSpAsync(&quot;Function async invoked using smart pointer. Bug solved!&quot;, 2016);
delegateMemberSpAsync.Clear();
testClassSp.reset();
</pre>

<p>Actually, this technique can be used to call an object function, and then the object automatically deletes after the callback occurs. Using the above example, create a shared pointer instance, bind a delegate, and invoke the delegate. Now <code>testClassSp </code>can go out of scope and <code>TestClass::MemberFuncStdString</code> will still be safely called on <code>workerThread1</code>. The <code>TestClass </code>instance will delete by way of <code>std::shared_ptr&lt;TestClass&gt;</code> once the smart pointer reference count goes to <code>0</code> after the callback completes without any extra programmer involvement.</p>

<pre lang="C++">
std::shared_ptr&lt;TestClass&gt; testClassSp(new TestClass());
auto delegateMemberSpAsync = 
    MakeDelegate(testClassSp, &amp;TestClass::MemberFuncStdString, &amp;workerThread1);
delegateMemberSpAsync(&quot;testClassSp deletes after delegate invokes&quot;, 2016);
</pre>

<h3>Asynchronous Blocking Delegates</h3>

<p>A blocking delegate waits until the target thread executes the bound delegate function. Unlike non-blocking delegates, the blocking versions do not copy argument data onto the heap. They also allow function return types other than <code>void </code>whereas the non-blocking delegates only bind to functions returning <code>void</code>. Since the function arguments are passed to the destination thread unmodified, the function executes just as you&#39;d expect a synchronous version including incoming/outgoing pointers and references.</p>

<p>Stack arguments passed by pointer/reference need not be thread-safe. The reason is that the calling thread blocks waiting for the destination thread to complete. This means that the delegate implementation guarantees only one thread is able to access stack allocated argument data.</p>

<p>A blocking delegate must specify a timeout in milliseconds or <code>WAIT_INFINITE</code>. Unlike a non-blocking asynchronous delegate, which is guaranteed to be invoked, if the timeout expires on a blocking delegate, the function is not invoked. Use <code>IsSuccess()</code> to determine if the delegate succeeded or not.</p>

<p>Adding a timeout as the last argument to <code>MakeDelegate()</code> causes a <code>DelegateFreeAsyncWaitX&lt;&gt;</code> or <code>DelegateMemberAsyncWaitX&lt;&gt;</code> instance to be returned depending on if a free or member function is being bound. A &quot;<code>Wait</code>&quot; delegate is typically not added to a delegate container. The typical usage pattern is to create a delegate and function arguments on the stack, then invoke. The code fragment below creates a blocking delegate with the function signature <code>int (std::string&amp;)</code>. The function is called on <code>workerThread1</code>. The function <code>MemberFuncStdStringRetInt()</code> will update the outgoing string <code>msg </code>and return an integer to the caller.</p>

<pre lang="C++">
DelegateMemberAsyncWait1&lt;TestClass, std::string&amp;, int&gt; delegateI =
              MakeDelegate(&amp;testClass, &amp;TestClass::MemberFuncStdStringRetInt, 
                           &amp;workerThread1, WAIT_INFINITE);
std::string msg;
int year = delegateI(msg);
if (delegateI.IsSuccess())
    cout &lt;&lt; msg.c_str() &lt;&lt; &quot; &quot; &lt;&lt; year &lt;&lt; endl;</pre>

<p>Using the keyword <code>auto </code>with delegates simplifies the syntax considerably.</p>

<pre lang="C++">
auto delegateI = 
    MakeDelegate(&amp;testClass, &amp;TestClass::MemberFuncStdStringRetInt, 
                 &amp;workerThread1, WAIT_INFINITE);
std::string msg;
int year = delegateI(msg);
if (delegateI.IsSuccess())
    cout &lt;&lt; msg.c_str() &lt;&lt; &quot; &quot; &lt;&lt; year &lt;&lt; endl;</pre>

<h2>Delegate Library</h2>

<p>The delegate library contains numerous classes. A single include <em>DelegateLib.h</em> provides access to all delegate library features. The defines within <em>DelegateOpt.h</em> set the library options. The library is wrapped within a <code>DelegateLib </code>namespace. Included unit tests help ensure a robust implementation. The table below shows the delegate class hierarchy.</p>

<ul class="class">
	<li style="margin-left: 40px"><code>DelegateBase</code></li>
	<li style="margin-left: 80px"><code>Delegate0&lt;&gt;</code></li>
	<li style="margin-left: 120px"><code>DelegateFree0&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateFreeAsync0&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateFreeAsyncWaitBase0&lt;&gt;</code></li>
	<li style="margin-left: 200px"><code>DelegateFreeAsyncWait0&lt;&gt;</code></li>
	<li style="margin-left: 120px"><code>DelegateMember0&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateMemberAsync0&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateMemberAsyncWaitBase0&lt;&gt;</code></li>
	<li style="margin-left: 200px"><code>DelegateMemberAsyncWait0&lt;&gt;</code></li>
	<li style="margin-left: 120px"><code>DelegateMemberSp0&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateMemberSpAsync0&lt;&gt;</code></li>
	<li style="margin-left: 80px"><code>Delegate1&lt;&gt;</code></li>
	<li style="margin-left: 120px"><code>DelegateFree1&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateFreeAsync1&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateFreeAsyncWaitBase0&lt;&gt;</code></li>
	<li style="margin-left: 200px"><code>DelegateFreeAsyncWait0&lt;&gt;</code></li>
	<li style="margin-left: 120px"><code>DelegateMember1&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateMemberAsync1&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateMemberAsyncWaitBase1&lt;&gt;</code></li>
	<li style="margin-left: 200px"><code>DelegateMemberAsyncWait1&lt;&gt;</code></li>
	<li style="margin-left: 120px"><code>DelegateMemberSp1&lt;&gt;</code></li>
	<li style="margin-left: 160px"><code>DelegateMemberSpAsync1&lt;&gt;</code></li>
</ul>

<p style="margin-left: 40px">etc...</p>

<p>Throughout the following discussion, I&rsquo;ll be using the one parameter version of the delegates.</p>

<p><code>DelegateBase</code> is a non-template, abstract base class common to all delegate instances. Comparison operators and a <code>Clone()</code> method define the interface.</p>

<pre lang="C++">
class DelegateBase {
public:
      virtual ~DelegateBase() {}

      /// Derived class must implement operator== to compare objects.
      virtual bool operator==(const DelegateBase&amp; rhs) const = 0;
      virtual bool operator!=(const DelegateBase&amp; rhs) { return !(*this == rhs); }


      /// Use Clone to provide a deep copy using a base pointer. Covariant
      /// overloading is used so that a Clone() method return type is a
      /// more specific type in the derived class implementations.
      /// @return A dynamic copy of this instance created with operator new.
      /// @post The caller is responsible for deleting the clone instance.
      virtual DelegateBase* Clone() const = 0;
};</pre>

<p><code>Delegate1&lt;&gt;</code> provides a template class with templatized function arguments. The <code>operator()</code> function allows invoking the delegate function with the correct function parameters. Covariant overloading of <code>Clone()</code> provides a more specific return type.</p>

<p>The <code>Clone()</code> function is required by the delegate container classes. The delegate container needs to make copies of the delegate for storage into the list. Since the delegate container only knows about abstract base <code>Delegate1&lt;&gt;</code> instances, it must use the <code>Clone()</code> function when creating a duplicate copy.</p>

<pre lang="C++">
template &lt;class Param1, class RetType=void&gt;
class Delegate1 : public DelegateBase {
public:
      virtual RetType operator()(Param1 p1) const = 0;
      virtual Delegate1&lt;Param1, RetType&gt;* Clone() const = 0;
};</pre>

<p>Efficiently storing instance member functions and free functions within the same class proves difficult. Instead, two classes were created for each type of bound function. <code>DelegateMember1&lt;&gt;</code> handles instance member functions. <code>DelegateFree1&lt;&gt;</code> handles free and static functions.</p>

<p><code>DelegateMember1&lt;&gt;</code> binds to an instance member function. The addition of a <code>TClass</code> template argument is added above the <code>Param1</code> and <code>RetType</code> required of the inherited <code>Delegate1&lt;&gt;</code> base class. <code>TClass</code> is required for binding to an instance member function. The delegate containers, however, cannot know about <code>TClass</code>. The container list may only store the most common ancestor of <code>DelegateMember1&lt;&gt;</code> and <code>DelegateFree1&lt;&gt;</code>, which happens to be the <code>Delegate1&lt;&gt;</code> interface.</p>

<p><code>Clone()</code> creates a new instance of the class. <code>Bind()</code> takes a class instance and a member function pointer. The function <code>operator()</code> allows invoking the delegate function assigned with <code>Bind()</code>.</p>

<pre lang="C++">
template &lt;class TClass, class Param1, class RetType=void&gt; 
class DelegateMember1 : public Delegate1&lt;Param1, RetType&gt; {
public:
    typedef TClass* ObjectPtr;
    typedef RetType (TClass::*MemberFunc)(Param1); 
    typedef RetType (TClass::*ConstMemberFunc)(Param1) const; 

    DelegateMember1(ObjectPtr object, MemberFunc func) { Bind(object, func); }
    DelegateMember1(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);    }
    DelegateMember1() :    m_object(0), m_func(0) { }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func) {
        m_object = object;
        m_func = func; }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func)    {
        m_object = object;
        m_func = reinterpret_cast&lt;MemberFunc&gt;(func); }

    virtual DelegateMember1* Clone() const { return new DelegateMember1(*this); }

    // Invoke the bound delegate function
    virtual RetType operator()(Param1 p1) {
        return (*m_object.*m_func)(p1); }

    virtual bool operator==(const DelegateBase&amp; rhs) const     {
        const DelegateMember1&lt;TClass, Param1, RetType&gt;* derivedRhs = 
            dynamic_cast&lt;const DelegateMember1&lt;TClass, Param1, RetType&gt;*&gt;(&amp;rhs);
        return derivedRhs &amp;&amp;
            m_func == derivedRhs-&gt;m_func &amp;&amp; 
            m_object == derivedRhs-&gt;m_object; }

    bool Empty() const { return !(m_object &amp;&amp; m_func); }
    void Clear() { m_object = 0; m_func = 0; }

    // New-school safe bool
    explicit operator bool() const { return !Empty();  }

private:
    ObjectPtr m_object;        // Pointer to a class object
    MemberFunc m_func;       // Pointer to an instance member function
};</pre>

<p><code>DelegateFree1&lt;&gt;</code> binds to a free or static member function. Notice it inherits from <code>Delegate1&lt;&gt;</code> just like <code>DelegateMember1&lt;&gt;</code>. <code>Bind()</code> takes a function pointer and <code>operator()</code> allows subsequent invocation of the bound function.</p>

<pre lang="C++">
template &lt;class Param1, class RetType=void&gt; 
class DelegateFree1 : public Delegate1&lt;Param1, RetType&gt; {
public:
    typedef RetType (*FreeFunc)(Param1); 

    DelegateFree1(FreeFunc func) { Bind(func); }
    DelegateFree1() : m_func(0) { }

    /// Bind a free function to the delegate.
    void Bind(FreeFunc func) { m_func = func; }

    virtual DelegateFree1* Clone() const { return new DelegateFree1(*this); }

    /// Invoke the bound delegate function. 
    virtual RetType operator()(Param1 p1) {
        return (*m_func)(p1); }

    virtual bool operator==(const DelegateBase&amp; rhs) const {
        const DelegateFree1&lt;Param1, RetType&gt;* derivedRhs = 
            dynamic_cast&lt;const DelegateFree1&lt;Param1, RetType&gt;*&gt;(&amp;rhs);
        return derivedRhs &amp;&amp;
            m_func == derivedRhs-&gt;m_func; }

    bool Empty() const { return !m_func; }
    void Clear() { m_func = 0; }

    // New-school safe bool
    explicit operator bool() const { return !Empty();  }

private:
    FreeFunc m_func;        // Pointer to a free function
};</pre>

<p><code>DelegateMemberAsync1&lt;&gt;</code> is the non-blocking asynchronous version of the delegate allowing invocation on a client specified thread of control. The <code>operator()</code> function doesn&rsquo;t actually call the target function, but instead packages the delegate and all function arguments onto the heap into a <code>DelegateMsg1&lt;&gt;</code> instance for sending through the message queue using <code>DispatchDelegate()</code>.</p>

<pre lang="C++">
template &lt;class TClass, class Param1&gt; 
class DelegateMemberAsync1 : public DelegateMember1&lt;TClass, Param1&gt;, public IDelegateInvoker {
public:
    // Constructors take a class instance, member function, and callback thread
    DelegateMemberAsync1(ObjectPtr object, MemberFunc func, DelegateThread* thread) { 
        Bind(object, func, thread); }
    DelegateMemberAsync1(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { 
        Bind(object, func, thread); }
    DelegateMemberAsync1() : m_thread(0) { }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
        m_thread = thread; 
        DelegateMember1&lt;TClass, Param1&gt;::Bind(object, func); }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)    {
        m_thread = thread;
        DelegateMember1&lt;TClass, Param1&gt;::Bind(object, func); }

    virtual DelegateMemberAsync1&lt;TClass, Param1&gt;* Clone() const {
        return new DelegateMemberAsync1&lt;TClass, Param1&gt;(*this); }

    virtual bool operator==(const DelegateBase&amp; rhs) const     {
        const DelegateMemberAsync1&lt;TClass, Param1&gt;* derivedRhs = 
            dynamic_cast&lt;const DelegateMemberAsync1&lt;TClass, Param1&gt;*&gt;(&amp;rhs);
        return derivedRhs &amp;&amp;
            m_thread == derivedRhs-&gt;m_thread &amp;&amp; 
            DelegateMember1&lt;TClass, Param1&gt;::operator == (rhs); }

    /// Invoke delegate function asynchronously
    virtual void operator()(Param1 p1) {
        if (m_thread == 0)
            DelegateMember1&lt;TClass, Param1&gt;::operator()(p1);
        else
        {
            // Create a new instance of the function argument data and copy
            Param1 heapParam1 = DelegateParam&lt;Param1&gt;::New(p1);

            // Create a clone instance of this delegate 
            DelegateMemberAsync1&lt;TClass, Param1&gt;* delegate = Clone();

            // Create a new message instance 
            DelegateMsg1&lt;Param1&gt;* msg = new DelegateMsg1&lt;Param1&gt;(delegate, heapParam1);

            // Dispatch message onto the callback destination thread. DelegateInvoke()
            // will be called by the target thread. 
            m_thread-&gt;DispatchDelegate(msg);
        }
    }

    /// Called by the target thread to invoke the delegate function 
    virtual void DelegateInvoke(DelegateMsgBase** msg) {
        // Typecast the base pointer to back to the templatized instance
        DelegateMsg1&lt;Param1&gt;* delegateMsg = static_cast&lt;DelegateMsg1&lt;Param1&gt;*&gt;(*msg);

        // Get the function parameter data
        Param1 param1 = delegateMsg-&gt;GetParam1();

        // Invoke the delegate function
        DelegateMember1&lt;TClass, Param1&gt;::operator()(param1);

        // Delete heap data created inside operator()
        DelegateParam&lt;Param1&gt;::Delete(param1);
        delete *msg;
        *msg = 0;

        // Do this last before returning!
        delete this;
    }

private:
    /// Target thread to invoke the delegate function
    DelegateThread* m_thread;
};</pre>

<p>Arguments come in different styles: by value, by reference, pointer and pointer to pointer. For non-blocking delegates, anything other than pass by value needs to have the data pointed to created on the heap to ensure the data is valid on the destination thread. The key to being able to save each parameter into <code>DelegateMsg1&lt;&gt;</code> is the <code>DelegateParam&lt;&gt;</code> class as used within the <code>operator()</code> function below.</p>

<pre lang="C++">
/// Invoke delegate function asynchronously
virtual void operator()(Param1 p1) {
    if (m_thread == 0)
        DelegateMember1&lt;TClass, Param1&gt;::operator()(p1);
    else
    {
        // Create a new instance of the function argument data and copy
        Param1 heapParam1 = DelegateParam&lt;Param1&gt;::New(p1);

        // Create a clone instance of this delegate 
        DelegateMemberAsync1&lt;TClass, Param1&gt;* delegate = Clone();

        // Create a new message instance 
        DelegateMsg1&lt;Param1&gt;* msg = new DelegateMsg1&lt;Param1&gt;(delegate, heapParam1);

        // Dispatch message onto the callback destination thread. DelegateInvoke()
        // will be called by the target thread. 
        m_thread-&gt;DispatchDelegate(msg);
    }
}</pre>

<p><code>DelegateMemberSpAsync1&lt;&gt;</code> is an non-blocking asychronous delegate that binds to a <code>std::shared_ptr</code> instead of a raw object pointer. The implementation is the same as the non-Sp version, except all locations of <code>TClass*</code> are replaced with <code>std::shared_ptr&lt;TClass&gt;</code>.</p>

<pre lang="C++">
template &lt;class TClass, class Param1, class RetType=void&gt; 
class DelegateMemberSp1 : public Delegate1&lt;Param1, RetType&gt; {
public:
    typedef std::shared_ptr&lt;TClass&gt; ObjectPtr;
    typedef RetType (TClass::*MemberFunc)(Param1); 
    typedef RetType (TClass::*ConstMemberFunc)(Param1) const; 

    DelegateMemberSp1(ObjectPtr object, MemberFunc func) { Bind(object, func); }
    DelegateMemberSp1(ObjectPtr object, ConstMemberFunc func) { Bind(object, func); }
    DelegateMemberSp1() : m_object(0), m_func(0) { }

etc...</pre>

<p><code>DelegateMemberAsyncWait1&lt;&gt;</code> is a blocking asynchronous delegate that binds to a class instance member function. <code>DelegateMemberAsyncWait1&lt;&gt;</code> has a template specialization for <code>void</code> return types so that functions with and without return values are supported. A common base class <code>DelegateMemberAsyncWaitBase1&lt;&gt;</code> are shared by the specializations. The two key functions are shown below. Notice that the implementation requires a semaphore to block the calling thread and a software lock to protect shared data.</p>

<pre lang="C++">
/// Invoke delegate function asynchronously
virtual RetType operator()(Param1 p1) {
    if (this-&gt;m_thread == 0)
        return DelegateMemberAsyncWaitBase1&lt;TClass, Param1, RetType&gt;::operator()(p1);
    else {
        // Create a clone instance of this delegate 
        DelegateMemberAsyncWait1&lt;TClass, Param1, RetType&gt;* delegate = Clone();
        delegate-&gt;m_refCnt = 2;
        delegate-&gt;m_sema.Create();
        delegate-&gt;m_sema.Reset();

        // Create a new message instance 
        DelegateMsg1&lt;Param1&gt;* msg = new DelegateMsg1&lt;Param1&gt;(delegate, p1);

        // Dispatch message onto the callback destination thread. DelegateInvoke()
        // will be called by the target thread. 
        this-&gt;m_thread-&gt;DispatchDelegate(msg);

        // Wait for target thread to execute the delegate function
        if ((this-&gt;m_success = delegate-&gt;m_sema.Wait(this-&gt;m_timeout)))
            m_retVal = delegate-&gt;m_retVal;

        bool deleteData = false;
        {
            LockGuard lockGuard(&amp;delegate-&gt;m_lock);
            if (--delegate-&gt;m_refCnt == 0)
                deleteData = true;
        }
        if (deleteData) {
            delete msg;
            delete delegate;
        }
        return m_retVal;
    }
}

/// Called by the target thread to invoke the delegate function 
virtual void DelegateInvoke(DelegateMsgBase** msg) {
    bool deleteData = false;
    {
        // Typecast the base pointer to back to the templatized instance
        DelegateMsg1&lt;Param1&gt;* delegateMsg = static_cast&lt;DelegateMsg1&lt;Param1&gt;*&gt;(*msg);

        // Get the function parameter data
        Param1 param1 = delegateMsg-&gt;GetParam1();

        LockGuard lockGuard(&amp;this-&gt;m_lock);
        if (this-&gt;m_refCnt == 2) {
            // Invoke the delegate function then signal the waiting thread
            m_retVal = 
              DelegateMemberAsyncWaitBase1&lt;TClass, Param1, RetType&gt;::operator()(param1);
            this-&gt;m_sema.Signal();
        }

        // If waiting thread is no longer waiting then delete heap data
        if (--this-&gt;m_refCnt == 0)
            deleteData = true;
    }
    if (deleteData) {
        delete *msg;
        *msg = 0;
        delete this;
    }
}</pre>

<h3>Argument Heap Copy</h3>

<p>Non-blocking asynchronous invocations means that all argument data must be copied into the heap for transport to the destination thread. The <code>DelegateParam&lt;&gt;</code> class is used to <code>new</code>/<code>delete</code> arguments. Template specialization is used to define different versions of <code>DelegateParam&lt;&gt;</code> based on the argument type: pass by value, reference, pointer, pointer to pointer. The snippet below shows how it&rsquo;s used to make a copy of function argument <code>p1</code> on the heap.</p>

<pre lang="C++">
// Create a new instance of the function argument data and copy
Param1 heapParam1 = DelegateParam&lt;Param1&gt;::New(p1);</pre>

<p>The actual <code>New()</code> function implementation called above depends on the <code>Param1</code> argument type. Pass by value will call the template version shown below. It actually doesn&rsquo;t create anything on the heap but instead just returns back the callers input value. The reason is that pass by value doesn&rsquo;t need a heap copy as it already is a copy. <code>Delete()</code> therefore does nothing as there is no data to delete.</p>

<pre lang="C++">
template &lt;typename Param&gt;
class DelegateParam
{
public:
      static Param New(Param param) { return param; }
      static void Delete(Param param) { }
};</pre>

<p>The <code>DelegateParam&lt;Param *&gt;</code> template specialization below handles all pointer type arguments. Unlike pass by value, a pointer points at something. That something must be created on the heap so the destination thread has a full copy upon callback invocation. <code>New()</code> creates the copy, and <code>Delete()</code> deletes it.</p>

<pre lang="C++">
template &lt;typename Param&gt;
class DelegateParam&lt;Param *&gt;
{
public:
      static Param* New(Param* param) {
            Param* newParam = new Param(*param);
            return newParam;
      }


      static void Delete(Param* param) {
            delete param;
      }
};</pre>

<p>Similarly, there are template specializations that handle references and pointers to pointers. This way, no matter the argument type, the delegate library behaves in a consistent and correct way with no awareness or special effort on the user&#39;s part.</p>

<h3>Bypassing Argument Heap Copy</h3>

<p>Occasionally, you may not want the delegate library to copy your pointer/reference arguments. Instead, you just want the destination thread to have a pointer to the original copy. Maybe the object is large or can&rsquo;t be copied. Or maybe it&rsquo;s a static instance that is guaranteed to exist. Either way, here is how to really send a pointer without duplicating the object pointed to.</p>

<p>The trick is to define a <code>DelegateParam&lt;&gt;</code> template specialization to handle your specific class/struct. In the example below, the structure <code>TestStructNoCopy</code> will not be copied by the delegate library. The <code>New()</code> function just returns the original pointer, and <code>Delete()</code> does nothing. Now, any <code>TestStructNoCopy*</code> delegate function arguments will use your <code>New()</code>/<code>Delete()</code> and not the library&rsquo;s default implementation.</p>

<pre lang="C++">
template &lt;&gt;
class DelegateParam&lt;TestStructNoCopy *&gt;
{
public:
      static TestStructNoCopy* New(TestStructNoCopy* param) { return param; }
      static void Delete(TestStructNoCopy* param) {}
};</pre>

<p>Using this technique means that the pointer you&rsquo;re passing must exist when the destination thread actually invokes the callback function. In addition, if multiple threads are accessing that instance the code within the class needs to be thread-safe.</p>

<p>This method is not required on blocking delegates, as the arguments are not copied.</p>

<h3>Array Argument Heap Copy</h3>

<p>Array function arguments are adjusted to a pointer per the C standard. In short, any function parameter declared as <code>T a[]</code> or <code>T a[N]</code> is treated as though it were declared as <code>T *a</code>. This means by default the delegate library <code>DelegateParam&lt;Param *&gt;</code> is called for array type parameters. Since the array size is not known, the <code>DelegateParam&lt;Param *&gt;</code> will only copy the first array element which is certainly not what is expected or desired. For instance, the function below:</p>

<pre lang="C++">
void ArrayFunc(char a[]) {}</pre>

<p>Requires a delegate argument <code>char*</code> because the <code>char a[]</code> was &ldquo;adjusted&rdquo; to <code>char *a</code>.</p>

<pre lang="C++">
MulticastDelegateSafe1&lt;char*&gt; delegateArrayFunc;
delegateArrayFunc += MakeDelegate(&amp;ArrayFunc, &amp;workerThread1);</pre>

<p>There is no way to asynchronously pass a C-style array by value. The best that can be achieved is to pass the array by pointer using the previously described template specialization technique. The class below passes every <code>char*</code>, <code>char a[]</code> or <code>char a[N]</code> as a <code>char*</code> and the array pointer will be passed to the invoked function without attempting a copy. Remember, it is up to you to ensure the pointer remains valid on the destination thread.</p>

<pre lang="C++">
template &lt;&gt;
class DelegateParam&lt;char *&gt;
{
public:
      static char* New(char* param) { return param; }
      static void Delete(char* param) {}
};</pre>

<p>My recommendation is to avoid C-style arrays if possible when using asynchronous delegates to avoid confusion and mistakes.</p>

<h3>Worker Thread (Win32)</h3>

<p>After the <code>operator()</code> function completes and the <code>DelegateMsg1&lt;&gt;</code> is put into the message queue, eventually <code>WorkerThread::Process()</code> will call <code>DelegateInvoke()</code> on the destination thread. The Win32 thread loop code below is from <em>WorkerThreadWin.cpp/h</em>.</p>

<pre lang="C++">
unsigned long WorkerThread::Process(void* parameter)
{
&nbsp;&nbsp; &nbsp;MSG msg;
&nbsp;&nbsp; &nbsp;BOOL bRet;

&nbsp;&nbsp; &nbsp;// Start periodic timer
&nbsp;&nbsp; &nbsp;MMRESULT timerId = timeSetEvent(100, 10, &amp;WorkerThread::TimerExpired, reinterpret_cast&lt;DWORD&gt;(this), TIME_PERIODIC);

&nbsp;&nbsp; &nbsp;while ((bRet = GetMessage(&amp;msg, NULL, WM_USER_BEGIN, WM_USER_END)) != 0)
&nbsp;&nbsp; &nbsp;{
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;switch (msg.message)
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;{
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;case WM_DISPATCH_DELEGATE:
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;{
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;ASSERT_TRUE(msg.wParam != NULL);

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;// Get the ThreadMsg from the wParam value
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;ThreadMsg* threadMsg = reinterpret_cast&lt;ThreadMsg*&gt;(msg.wParam);

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;// Convert the ThreadMsg void* data back to a DelegateMsg*&nbsp;
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;DelegateMsgBase* delegateMsg = static_cast&lt;DelegateMsgBase*&gt;(threadMsg-&gt;GetData());&nbsp;

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;// Invoke the callback on the target thread
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;delegateMsg-&gt;GetDelegateInvoker()-&gt;DelegateInvoke(&amp;delegateMsg);

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;// Delete dynamic data passed through message queue
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;delete threadMsg;
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;break;
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;}

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;case WM_USER_TIMER:
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;Timer::ProcessTimers();
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;break;

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;case WM_EXIT_THREAD:
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;timeKillEvent(timerId);
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;return 0;

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;default:
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;ASSERT();
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;}
&nbsp;&nbsp; &nbsp;}
&nbsp;&nbsp; &nbsp;return 0;
}</pre>

<p>Notice the thread loop is unlike most systems that have a huge <code>switch</code> statement handling various incoming data messages, type casting <code>void*</code> data, then calling a specific function. The framework supports all delegate invocations with a single <code>WM_DISPATCH_DELEGATE</code> message. Once setup, the same small thread loop handles every delegate. New asynchronous delegates come and go as the system is designed, but the code in-between doesn&#39;t change.</p>

<p>This is a huge benefit as on many systems, getting data between threads takes a lot of manual steps. You constantly have to mess with each thread loop, create during sending, destroy data when receiving, and call various OS services and typecasts. Here, you do none of that. All the stuff in-between is neatly handled for users.</p>

<p>The <code>DelegateMemberAsync1&lt;&gt;::DelegateInvoke()</code> function calls the target function and deletes the data that traveled through the message queue inside <code>DelegateMsg1&lt;&gt;</code>. The delegate deletes all heap data and itself before returning.</p>

<pre lang="C++">
virtual void DelegateInvoke(DelegateMsgBase** msg) const {
        // Typecast the base pointer to back to the templatized instance
        DelegateMsg1&lt;Param1&gt;* delegateMsg = static_cast&lt;DelegateMsg1&lt;Param1&gt;*&gt;(*msg);

        // Get the function parameter data
        Param1 param1 = delegateMsg-&gt;GetParam1();

        // Invoke the delegate function
        DelegateMember1&lt;TClass, Param1, RetType&gt;::operator()(param1);

        // Delete heap data created inside operator()
        DelegateParam&lt;Param1&gt;::Delete(param1);
        delete *msg;
        *msg = 0;

        // Do this last before returning!
        delete this;
}</pre>

<h3>Worker Thread (std::thread)</h3>

<p>Instead of the Win32 API, an alternate implementation using the <code>std::thread</code> classes is included. Any C++11 compiler that supports <code>std::thread</code> is able to build and use the delegate library. Within <em>DelegateOpt.h</em>, define <code>USE_STD_THREADS</code> instead of <code>USE_WIN32_THREADS</code> to use the <code>WorkerThread</code> class contained within <em>WorkerThreadStd.cpp/h</em>. The <code>LockGuard</code> and <code>Semaphore </code>classes are also conditionally compiled to use the C++ Standard Library instead of the Win32 API. The <code>std::thread</code> implemented thread loop is shown below.</p>

<pre lang="C++">
void WorkerThread::Process()
{
&nbsp; &nbsp; m_timerExit = false;
&nbsp; &nbsp; std::thread timerThread(&amp;WorkerThread::TimerThread, this);

&nbsp;&nbsp; &nbsp;while (1)
&nbsp;&nbsp; &nbsp;{
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;std::shared_ptr&lt;ThreadMsg&gt; msg;
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;{
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;// Wait for a message to be added to the queue
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;std::unique_lock&lt;std::mutex&gt; lk(m_mutex);
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;while (m_queue.empty())
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;m_cv.wait(lk);

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;if (m_queue.empty())
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;continue;

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;msg = m_queue.front();
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;m_queue.pop();
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;}

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;switch (msg-&gt;GetId())
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;{
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;case MSG_DISPATCH_DELEGATE:
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;{
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;ASSERT_TRUE(msg-&gt;GetData() != NULL);

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;// Convert the ThreadMsg void* data back to a DelegateMsg*&nbsp;
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; auto delegateMsg = static_cast&lt;DelegateMsgBase*&gt;(msg-&gt;GetData());

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;// Invoke the callback on the target thread
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;delegateMsg-&gt;GetDelegateInvoker()-&gt;DelegateInvoke(&amp;delegateMsg);
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;break;
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;}

&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; case MSG_TIMER:
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; Timer::ProcessTimers();
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; break;

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;case MSG_EXIT_THREAD:
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;{
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; m_timerExit = true;
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; timerThread.join();
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; return;
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;}

&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;default:
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;ASSERT();
&nbsp;&nbsp; &nbsp;&nbsp;&nbsp; &nbsp;}
&nbsp;&nbsp; &nbsp;}
}</pre>

<h2>Delegate Invocation</h2>

<p>A bound delegate function is invoked with the function <code>operator()</code>. When invoking a delegate from a container, three function calls are required. One non-virtual <code>operator()</code> within the delegate container, a second <code>virtual</code> <code>operator()</code> on the delegate, then finally the bound function is called.</p>

<p>For a multicast delegate, the container <code>operator()</code> function iterates over the list calling each delegate&rsquo;s <code>operator()</code>. Notice, there is no return value when executing a delegate function within a multicast delegate container.</p>

<pre lang="C++">
template&lt;typename Param1&gt;
class MulticastDelegate1 : public MulticastDelegateBase
{
public:
    void operator()(Param1 p1) {
        InvocationNode* node = GetInvocationHead();
        while (node != 0) {
            const Delegate1&lt;Param1, void&gt;* delegate = 
                static_cast&lt;const Delegate1&lt;Param1, void&gt;*&gt;(node-&gt;Delegate);
            (*delegate)(p1);    // Invoke delegate callback
            node = node-&gt;Next;
        }
    }
};</pre>

<p>For a single cast delegate, the delegate container <code>operator()</code> just calls the delegate <code>operator()</code>. Notice that a return value is permitted using the single cast container.</p>

<pre lang="C++">
RetType operator()(Param p1) {
      return (*m_delegate)(p1); } // Invoke delegate function</pre>

<p>The containers don&rsquo;t have awareness of the concrete delegate types. If the delegate stored within the container is the synchronous version, the bound function is called synchronously. When asynchronous delegates are invoked, the delegate and arguments are sent through a message queue to the destination thread.</p>

<p>When using a delegate without a container, the invocation is goes from the delegate <code>virtual operator()</code> to the bound function. The snippet below shows invoking an instance member function.</p>

<pre lang="C++">
// Invoke the bound delegate function
virtual RetType operator()(Param1 p1) const {
      return (*m_object.*m_func)(p1); }</pre>

<p>Obtaining the utmost speed from the delegate wasn&rsquo;t a priority especially when you involve a message queue; a few extra instructions aren&rsquo;t likely to matter much. A standard compliant delegate with the ability to derive additional functionality trumped any such optimizations.</p>

<h2>Delegate Containers</h2>

<p>Delegate containers store one or more delegates. The delegate container hierarchy is shown below:</p>

<ul class="class">
	<li style="margin-left: 40px"><code>MulticastDelegateBase</code></li>
	<li style="margin-left: 80px"><code>MulticastDelegate0</code></li>
	<li style="margin-left: 120px"><code>MulticastDelegateSafe0</code></li>
	<li style="margin-left: 80px"><code>MulticastDelegate1&lt;&gt;</code></li>
	<li style="margin-left: 120px"><code>MulticastDelegateSafe1&lt;&gt;</code></li>
</ul>

<p style="margin-left: 80px">etc...</p>

<ul class="class">
	<li style="margin-left: 40px"><code>SinglecastDelegate0&lt;&gt;</code></li>
	<li style="margin-left: 40px"><code>SinglecastDelegate1&lt;&gt;</code></li>
</ul>

<p style="margin-left: 40px">etc...</p>

<p>The <code>MulticastDelegateBase</code> provides a non-template base for storing non-template <code>DelegateBase</code> instances within a list. The <code>operator+=</code> add a delegate to the list and <code>operator-=</code> removes it.</p>

<pre lang="C++">
class MulticastDelegateBase
{
public:
    /// Constructor
    MulticastDelegateBase() : m_invocationHead(0) {}

    /// Destructor
    virtual ~MulticastDelegateBase() { Clear(); }

    /// Any registered delegates?
    bool Empty() const { return !m_invocationHead; }

    /// Removal all registered delegates.
    void Clear();

protected:
    struct InvocationNode
    {
        InvocationNode() : Next(0), Delegate(0) { }
        InvocationNode* Next;
        DelegateBase* Delegate;
    };

    /// Insert a delegate into the invocation list. A delegate argument 
    /// pointer is not stored. Instead, the DelegateBase derived object is 
    /// copied (cloned) and saved in the invocation list.
    /// @param[in] delegate - a delegate to register. 
    void operator+=(DelegateBase&amp; delegate);

    /// Remove a delegate previously registered delegate from the invocation
    /// list. 
    /// @param[in] delegate - a delegate to unregister. 
    void operator-=(DelegateBase&amp; delegate);

...</pre>

<p><code>MulticastDelegate1&lt;&gt;</code> provides the function <code>operator()</code> to sequentially invoke each delegate within the list. A simple cast is required to get the <code>DelegateBase</code> typed back to a more specific <code>Delegate1&lt;&gt;</code> instance.</p>

<pre lang="C++">
template&lt;typename Param1&gt;
class MulticastDelegate1 : public MulticastDelegateBase
{
public:
    MulticastDelegate1() { }
    void operator()(Param1 p1) {
        InvocationNode* node = GetInvocationHead();
        while (node != 0) {
            Delegate1&lt;Param1&gt;* delegate = 
                static_cast&lt;Delegate1&lt;Param1&gt;*&gt;(node-&gt;Delegate);
            (*delegate)(p1);    // Invoke delegate callback
            node = node-&gt;Next;
        }
    }
    void operator+=(Delegate1&lt;Param1&gt;&amp; delegate) 
                   { MulticastDelegateBase::operator+=(delegate); }
    void operator-=(Delegate1&lt;Param1&gt;&amp; delegate) 
                   { MulticastDelegateBase::operator-=(delegate); }

private:
    // Prevent copying objects
    MulticastDelegate1(const MulticastDelegate1&amp;);
    MulticastDelegate1&amp; operator=(const MulticastDelegate1&amp;);
};</pre>

<p><code>MulticastDelegateSafe1&lt;&gt;</code> provides a thread-safe wrapper around the <code>delegate</code> API. Each function provides a lock guard to protect against simultaneous access. The Resource Acquisition is Initialization (RAII) technique is used for the locks.</p>

<pre lang="C++">
template&lt;typename Param1&gt;
class MulticastDelegateSafe1 : public MulticastDelegate1&lt;Param1&gt;
{
public:
    MulticastDelegateSafe1() { LockGuard::Create(&amp;m_lock); }
    ~MulticastDelegateSafe1() { LockGuard::Destroy(&amp;m_lock); }

    void operator+=(Delegate1&lt;Param1&gt;&amp; delegate) { 
        LockGuard lockGuard(&amp;m_lock);
        MulticastDelegate1&lt;Param1&gt;::operator +=(delegate); 
    }
    void operator-=(Delegate1&lt;Param1&gt;&amp; delegate)    { 
        LockGuard lockGuard(&amp;m_lock);
        MulticastDelegate1&lt;Param1&gt;::operator -=(delegate); 
    }
    void operator()(Param1 p1) {
        LockGuard lockGuard(&amp;m_lock);
        MulticastDelegate1&lt;Param1&gt;::operator ()(p1); 
    }
    bool Empty() { 
        LockGuard lockGuard(&amp;m_lock);
        return MulticastDelegate1&lt;Param1&gt;::Empty();
    }
    void Clear() {
        LockGuard lockGuard(&amp;m_lock);
        return MulticastDelegate1&lt;Param1&gt;::Clear();
    }
    explicit operator bool() {
        LockGuard lockGuard(&amp;m_lock);
        return MulticastDelegateBase::operator bool();
    }

private:
    // Prevent copying objects
    MulticastDelegateSafe1(const MulticastDelegateSafe1&amp;);
    MulticastDelegateSafe1&amp; operator=(const MulticastDelegateSafe1&amp;);

    /// Lock to make the class thread-safe
    LOCK m_lock;
};</pre>

<h2>SysData Example</h2>

<p>A few real-world examples will demonstrate common delegate usage patterns. First, <code>SysData</code> is a simple class showing how to expose an outgoing asynchronous interface. The class stores system data and provides asynchronous subscriber notifications when the mode changes. The class interface is shown below.</p>

<pre lang="C++">
class SysData
{
public:
      /// Clients register with MulticastDelegateSafe1 
      /// to get callbacks when system mode changes
      MulticastDelegateSafe1&lt;const SystemModeChanged&amp;&gt; SystemModeChangedDelegate;

      /// Get singleton instance of this class
      static SysData&amp; GetInstance();

      /// Sets the system mode and notify registered clients via SystemModeChangedDelegate.
      /// @param[in] systemMode - the new system mode.
      void SetSystemMode(SystemMode::Type systemMode);     

private:
      SysData();
      ~SysData();

      /// The current system mode data
      SystemMode::Type m_systemMode;

      /// Lock to make the class thread-safe
      LOCK m_lock;
};</pre>

<p>The subscriber interface for receiving callbacks is <code>SystemModeChangedDelegate</code>. Calling <code>SetSystemMode()</code> saves the new mode into <code>m_systemMode</code> and notifies all registered subscribers.</p>

<pre lang="C++">
void SysData::SetSystemMode(SystemMode::Type systemMode)
{
      LockGuard lockGuard(&amp;m_lock);

      // Create the callback data
      SystemModeChanged callbackData;
      callbackData.PreviousSystemMode = m_systemMode;
      callbackData.CurrentSystemMode = systemMode;

      // Update the system mode
      m_systemMode = systemMode;

      // Callback all registered subscribers
      if (SystemModeChangedDelegate)
            SystemModeChangedDelegate(callbackData);
}</pre>

<h2>SysDataClient Example</h2>

<p><code>SysDataClient</code> is a delegate subscriber and registers for <code>SysData::SystemModeChangedDelegate</code> notifications within the constructor.</p>

<pre lang="C++">
// Constructor
SysDataClient() :
     m_numberOfCallbacks(0)
{
     // Register for async delegate callbacks
     SysData::GetInstance().SystemModeChangedDelegate += 
           MakeDelegate(this, &amp;SysDataClient::CallbackFunction, &amp;workerThread1);
     SysDataNoLock::GetInstance().SystemModeChangedDelegate += 
           MakeDelegate(this, &amp;SysDataClient::CallbackFunction, &amp;workerThread1);
}</pre>

<p><code>SysDataClient::CallbackFunction()</code> is now called on <code>workerThread1 </code>when the system mode changes.</p>

<pre lang="C++">
void CallbackFunction(const SystemModeChanged&amp; data)
{
    m_numberOfCallbacks++;
    cout &lt;&lt; &quot;CallbackFunction &quot; &lt;&lt; data.CurrentSystemMode &lt;&lt; endl;
}</pre>

<p>When <code>SetSystemMode()</code> is called, anyone interested in the mode changes are notified synchronously or asynchronously depending on the delegate type registered.</p>

<pre lang="C++">
// Set new SystemMode values. Each call will invoke callbacks to all
// registered client subscribers.
SysData::GetInstance().SetSystemMode(SystemMode::STARTING);
SysData::GetInstance().SetSystemMode(SystemMode::NORMAL);</pre>

<h2>SysDataNoLock Example</h2>

<p><code>SysDataNoLock</code> is an alternate implementation that uses a <code>private</code> <code>MulticastDelegateSafe1&lt;&gt;</code> for setting the system mode asynchronously and without locks.</p>

<pre lang="C++">
class SysDataNoLock
{
public:
    /// Clients register with MulticastDelegateSafe1 to get callbacks when system mode changes
    MulticastDelegateSafe1&lt;const SystemModeChanged&amp;&gt; SystemModeChangedDelegate;

    /// Get singleton instance of this class
    static SysDataNoLock&amp; GetInstance();

    /// Sets the system mode and notify registered clients via SystemModeChangedDelegate.
    /// @param[in] systemMode - the new system mode. 
    void SetSystemMode(SystemMode::Type systemMode);    

    /// Sets the system mode and notify registered clients via a temporary stack created
    /// asynchronous delegate. 
    /// @param[in] systemMode - The new system mode. 
    void SetSystemModeAsyncAPI(SystemMode::Type systemMode);    

    /// Sets the system mode and notify registered clients via a temporary stack created
    /// asynchronous delegate. This version blocks (waits) until the delegate callback
    /// is invoked and returns the previous system mode value. 
    /// @param[in] systemMode - The new system mode. 
    /// @return The previous system mode. 
    SystemMode::Type SetSystemModeAsyncWaitAPI(SystemMode::Type systemMode);

private:
    SysDataNoLock();
    ~SysDataNoLock();

    /// Private callback to get the SetSystemMode call onto a common thread
    MulticastDelegateSafe1&lt;SystemMode::Type&gt; SetSystemModeDelegate;

    /// Sets the system mode and notify registered clients via SystemModeChangedDelegate.
    /// @param[in] systemMode - the new system mode. 
    void SetSystemModePrivate(SystemMode::Type);    

    /// The current system mode data
    SystemMode::Type m_systemMode;
};</pre>

<p>The constructor registers <code>SetSystemModePrivate()</code> with the private <code>SetSystemModeDelegate</code>.</p>

<pre lang="C++">
SysDataNoLock::SysDataNoLock() :
      m_systemMode(SystemMode::STARTING)
{
      SetSystemModeDelegate += 
           MakeDelegate(this, &amp;SysDataNoLock::SetSystemModePrivate, &amp;workerThread2);
      workerThread2.CreateThread();
}</pre>

<p>The <code>SetSystemMode()</code> function below is an example of an asynchronous incoming interface. To the caller, it looks like a normal function, but under the hood, a private member call is invoked asynchronously using a delegate. In this case, invoking <code>SetSystemModeDelegate</code> causes <code>SetSystemModePrivate()</code> to be called on <code>workerThread2</code>.</p>

<pre lang="C++">
void SysDataNoLock::SetSystemMode(SystemMode::Type systemMode)
{
      // Invoke the private callback. SetSystemModePrivate() will be called on workerThread2.
      SetSystemModeDelegate(systemMode);
}</pre>

<p>Since this private function is always invoked asynchronously on <code>workerThread2</code>, it doesn&#39;t require locks.</p>

<pre lang="C++">
void SysDataNoLock::SetSystemModePrivate(SystemMode::Type systemMode)
{
      // Create the callback data
      SystemModeChanged callbackData;
      callbackData.PreviousSystemMode = m_systemMode;
      callbackData.CurrentSystemMode = systemMode;

      // Update the system mode
      m_systemMode = systemMode;

      // Callback all registered subscribers
      if (SystemModeChangedDelegate)
            SystemModeChangedDelegate(callbackData);
}</pre>

<h2>SysDataNoLock Reinvoke Example</h2>

<p>While creating a separate private function to create an asynchronous API does work, with delegates, it&#39;s possible to just reinvoke the same exact function just on a different thread. Perform a simple check whether the caller is executing on the desired thread of control. If not, a temporary asynchronous delegate is created on the stack and then invoked. The delegate and all the caller&rsquo;s original function arguments are duplicated on the heap and the function is reinvoked on <code>workerThread2</code>. This is an elegant way to create asynchronous APIs with the absolute minimum of effort.</p>

<pre lang="C++">
void SysDataNoLock::SetSystemModeAsyncAPI(SystemMode::Type systemMode)
{
    // Is the caller executing on workerThread2?
    if (workerThread2.GetThreadId() != WorkerThread::GetCurrentThreadId())
    {
        // Create an asynchronous delegate and re-invoke the function call on workerThread2
        auto delegate = 
             MakeDelegate(this, &amp;SysDataNoLock::SetSystemModeAsyncAPI, &amp;workerThread2);
        delegate(systemMode);
        return;
    }

    // Create the callback data
    SystemModeChanged callbackData;
    callbackData.PreviousSystemMode = m_systemMode;
    callbackData.CurrentSystemMode = systemMode;

    // Update the system mode
    m_systemMode = systemMode;

    // Callback all registered subscribers
    if (SystemModeChangedDelegate)
        SystemModeChangedDelegate(callbackData);
}</pre>

<h2>SysDataNoLock Blocking Reinvoke Example</h2>

<p>A blocking asynchronous API can be hidden inside a class member function. The function below sets the current mode on <code>workerThread2</code> and returns the previous mode. A blocking delegate is created on the stack and invoked if the caller isn&#39;t executing on <code>workerThread2</code>. To the caller, the function appears synchronous, but the delegate ensures that the call is executed on the proper thread before returning.</p>

<pre lang="C++">
SystemMode::Type SysDataNoLock::SetSystemModeAsyncWaitAPI(SystemMode::Type systemMode)
{
    // Is the caller executing on workerThread2?
    if (workerThread2.GetThreadId() != WorkerThread::GetCurrentThreadId())
    {
        // Create an asynchronous delegate and re-invoke the function call on workerThread2
        auto delegate = 
             MakeDelegate(this, &amp;SysDataNoLock::SetSystemModeAsyncWaitAPI, &amp;workerThread2, 
           WAIT_INFINITE);
        return delegate(systemMode);
    }

    // Create the callback data
    SystemModeChanged callbackData;
    callbackData.PreviousSystemMode = m_systemMode;
    callbackData.CurrentSystemMode = systemMode;

    // Update the system mode
    m_systemMode = systemMode;

    // Callback all registered subscribers
    if (SystemModeChangedDelegate)
        SystemModeChangedDelegate(callbackData);

    return callbackData.PreviousSystemMode;
}</pre>

<h2>Timer Example</h2>

<p>Once a delegate framework is in place, creating a timer callback service is trivial. Many systems need a way to generate a callback based on a timeout. Maybe it&#39;s a periodic timeout for some low speed polling or maybe an error timeout in case something doesn&#39;t occur within the expected time frame. Either way, the callback must occur on a specified thread of control. A <code>SinglecastDelegate0&lt;&gt;</code> used inside a <code>Timer</code> class solves this nicely.</p>

<pre lang="C++">
class Timer
{
public:
    SinglecastDelegate0&lt;&gt; Expired;

    void Start(UINT32 timeout);
    void Stop();

    //...
};</pre>

<p>Users create an instance of the timer and register for the expiration. In this case, <code>MyClass::MyCallback()</code> is called in 1000ms.</p>

<pre lang="C++">
m_timer.Expired = MakeDelegate(&amp;myClass, &amp;MyClass::MyCallback, &amp;myThread);
m_timer.Start(1000);</pre>

<h2>Heap vs. Fixed Block</h2>

<p>The heap is used to create duplicate copies of the delegate and function arguments. When adding a delegate to a multicast list, it is cloned using operator <code>new</code>. The asynchronous delegate support requires copying the delegate and all arguments for placement into a message queue. Normally, the memory comes from the heap. On many systems, this is not a problem. However, some system can&rsquo;t use the heap in an uncontrolled fashion due to the possibility of a heap fragmentation memory fault. This occurs when the heap memory gets chopped up into small blocks over long executions such that a memory request fails.</p>

<p>A fixed block memory allocator is included within the source files. Just uncomment the <code>USE_XALLOCATOR</code> define within <em>DelegateOpt.h</em> to enable using the fixed allocator. When enabled, all dynamic memory requests originating from the <code>delegate</code> library are routed to the fixed block allocators. The <code>xallocator</code> also has the advantage of faster execution than the heap thus limiting the speed impact of dynamic memory allocation.</p>

<p>The entire <code>delegate</code> hierarchy is routed to fixed block usage with a single <code>XALLOCATOR</code> macro inside <code>DelegateBase</code>.</p>

<pre lang="C++">
class DelegateBase {
#if USE_XALLOCATOR
      XALLOCATOR
#endif</pre>

<p>The <code>delegate</code> library copies function arguments when necessary for asynchronous support. The memory requests are routed to the fixed block allocator by way of the <code>DelegateParam&lt;&gt;</code> class. Notice that if <code>USE_ALLOCATOR</code> is defined, the memory is obtained within <code>New()</code> is from <code>xmalloc()</code> and the placement <code>new</code> syntax is used to construct the object within the fixed block region. Inside <code>Delete()</code>, the destructor is called manually then <code>xfree()</code> is used to return the fixed block memory.</p>

<pre lang="C++">
template &lt;typename Param&gt;
class DelegateParam&lt;Param *&gt;
{
public:
      static Param* New(Param* param) {
#if USE_XALLOCATOR
            void* mem = xmalloc(sizeof(*param));
            Param* newParam = new (mem) Param(*param);
#else
            Param* newParam = new Param(*param);
#endif
            return newParam;
      }

      static void Delete(Param* param) {
#if USE_XALLOCATOR
            param-&gt;~Param();
            xfree((void*)param);
#else
            delete param;
#endif
      }
};</pre>

<p>See the article &ldquo;<a href="https://www.codeproject.com/Articles/1084801/Replace-malloc-free-with-a-Fast-Fixed-Block-Memory"><strong>Replace malloc/free with a Fast Fixed Block Memory Allocator</strong></a>&rdquo; for more information.</p>

<h2>Porting</h2>

<p>The code is an easy port to any platform. There are only three OS services required: threads, a semaphore and a software lock. The code is separated into five directories.</p>

<ol>
	<li><em><strong>Delegate</strong></em> - core delegate library implementation files</li>
	<li><em><strong>Port</strong></em> &ndash; thread-specific files</li>
	<li><em><strong>Examples</strong></em> &ndash; sample code showing usage</li>
	<li><em><strong>VS2008</strong></em> &ndash; Visual Studio 2008 project files</li>
	<li><em><strong>VS2015</strong></em> &ndash; Visual Studio 2015 project files</li>
</ol>

<p>The Eclipse project files are located at the project root (<em>.cproject</em> and <em>.project</em>). Use the <strong>File</strong> &gt; <strong>Import...</strong> &gt; <strong>General</strong> &gt; <strong>Existing Projects into Workspace</strong> option to add the project to your workspace.</p>

<p>The library has a single <code>abstract</code> class <code>DelegateThread</code> with a single pure <code>virtual</code> function that needs to be implemented on each target OS.</p>

<pre>
virtual void DispatchDelegate(DelegateMsgBase* msg) = 0;</pre>

<p>On most projects, I wrap the underlying raw OS calls into a thread class to encapsulate and enforce the correct behavior. Here, I provide <code>ThreadWin</code> class as a wrapper over the <code>CreateThread()</code> Windows API.</p>

<p>Once you have a thread class, just inherit the <code>DelegateThread</code> interface and implement the <code>DispatchDelegate()</code> function. Using the Win32 API, a simple post to a message queue is all that is required:</p>

<pre lang="C++">
void ThreadWin::DispatchDelegate(DelegateMsgBase* msg)
{
      // Create a new ThreadMsg
      ThreadMsg* threadMsg = new ThreadMsg(WM_DISPATCH_DELEGATE, msg);

      // Post the message to the this thread&#39;s message queue
      PostThreadMessage(WM_DISPATCH_DELEGATE, threadMsg);
}</pre>

<p>The alternate implementation using the C++ Standard Library adds the message to a <code>std::queue</code> protected by a mutex.</p>

<pre>
void WorkerThread::DispatchDelegate(DelegateLib::DelegateMsgBase* msg)
{
&nbsp;&nbsp; &nbsp;ASSERT_TRUE(m_thread);

&nbsp;&nbsp; &nbsp;// Create a new ThreadMsg
&nbsp; &nbsp; std::shared_ptr&lt;ThreadMsg&gt; threadMsg(new ThreadMsg(MSG_DISPATCH_DELEGATE, msg));

&nbsp;&nbsp; &nbsp;// Add dispatch delegate msg to queue and notify worker thread
&nbsp;&nbsp; &nbsp;std::unique_lock&lt;std::mutex&gt; lk(m_mutex);
&nbsp;&nbsp; &nbsp;m_queue.push(threadMsg);
&nbsp;&nbsp; &nbsp;m_cv.notify_one();
}</pre>

<p>Software locks are handled by the <code>LockGuard</code> class. This class can be updated with locks of your choice, or you can use a different mechanism. Locks are only used in a few places. The <code>Semaphore</code> class wraps the Windows event objects or <code>std::mutex</code> required by the blocking delegate implementation.</p>

<p>In short, the library supports Win32 and <code>std::thread</code> models by defining <code>USE_WIN32_THREADS</code> or <code>USE_STD_THREADS</code> within <em>DelegateOpt.h</em>. If your C++11 or higher compiler supports <code>std::thread</code>, then you&#39;re good to go. For other OSs, just provide an implementation for <code>DelegateThread::DispatchDelegate()</code>, update the <code>LockGuard</code> and <code>Semaphore</code> classes, and put a small amount of code in your thread loop to call <code>DelegateInvoke()</code> and the <code>delegate</code> library can be deployed on any platform.</p>

<h2>Summary</h2>

<p>All delegates can be created with <code>MakeDelegate()</code>. The function arguments determine the delegate type returned.</p>

<p>Synchronous delegates are created using one argument for free functions and two for instance member functions.</p>

<pre lang="C++">
auto freeDelegate = MakeDelegate(&amp;MyFreeFunc);
auto memberDelegate = MakeDelegate(&amp;myClass, &amp;MyClass::MyMemberFunc);</pre>

<p>Adding the thread argument creates a non-blocking asynchronous delegate.</p>

<pre lang="C++">
auto freeDelegate = MakeDelegate(&amp;MyFreeFunc, &amp;myThread);
auto memberDelegate = MakeDelegate(&amp;myClass, &amp;MyClass::MyMemberFunc, &amp;myThread);</pre>

<p>If using C++11, a <code>std::shared_ptr</code> can replace a raw instance pointer on synchronous and non-blocking asynchronous member delegates.</p>

<pre lang="C++">
std::shared_ptr&lt;MyClass&gt; myClass(new MyClass());
auto memberDelegate = MakeDelegate(myClass, &amp;MyClass::MyMemberFunc, &amp;myThread);</pre>

<p>Adding a timeout argument creates a blocking asynchronous delegate.</p>

<pre lang="C++">
auto freeDelegate = MakeDelegate(&amp;MyFreeFunc, &amp;myThread, WAIT_INFINITE);
auto memberDelegate = MakeDelegate(&amp;myClass, &amp;MyClass::MyMemberFunc, &amp;myThread, 5000);</pre>

<p>Delegates are added/removed from multicast containers using <code>operator+=</code> and <code>operator-=</code>. All containers accept all delegate types.</p>

<pre lang="C++">
MulticastDelegate1&lt;int&gt; multicastContainer;
multicastContainer += MakeDelegate(&amp;MyFreeFunc);
multicastContainer -= MakeDelegate(&amp;MyFreeFunc);</pre>

<p>Use the thread-safe multicast delegate container when using asynchronous delegates to allow multiple threads to safely add/remove from the container.</p>

<pre lang="C++">
MulticastDelegateSafe1&lt;int&gt; multicastContainer;
multicastContainer += MakeDelegate(&amp;MyFreeFunc, &amp;myThread);
multicastContainer -= MakeDelegate(&amp;MyFreeFunc, &amp;myThread);</pre>

<p>Single cast delegates are added and removed using <code>operator=</code>.</p>

<pre lang="C++">
SinglecastDelegate1&lt;int&gt; singlecastContainer;
singlecastContainer = MakeDelegate(&amp;MyFreeFunc);
singlecastContainer = 0;</pre>

<p>All delegates and delegate containers are invoked using <code>operator()</code>.</p>

<pre lang="C++">
if (myDelegate)
      myDelegate(123);</pre>

<p>Use <code>IsSuccess()</code> on blocking delegates before using the return value or outgoing arguments.</p>

<pre lang="C++">
if (myDelegate) {
     int outInt = 0;
     int retVal = myDelegate(&amp;outInt);
     if (myDelegate.IsSuccess()) {
            cout &lt;&lt; outInt &lt;&lt; retVal;
     }        
}</pre>

<h2><a name="whichcallback">Which Callback Implementation?</a></h2>

<p>I&rsquo;ve documented four different asynchronous multicast callback implementations here on CodeProject. Each version has its own unique features and advantages. The sections below highlight the main differences between each solution. See the <strong>References </strong>section below for links to each article.</p>

<p>This asynchronous delegate implementation strives to ease inter-thread communication by invoking functions and passing data between threads using C++ delegates. Remote delegates extend the library to include inter-process and inter-processor communications.&nbsp;See the <strong>References </strong>section below for a the explaination of the remote procedure call using C++ delegates implementation.</p>

<h3>Asynchronous Multicast Callbacks in C</h3>

<ul>
	<li>Implemented in C</li>
	<li>Callback function is a free or static member only</li>
	<li>One callback argument supported</li>
	<li>Callback argument must be a pointer type</li>
	<li>Callback argument data copied with <code>memcpy</code></li>
	<li>Type-safety provided by macros</li>
	<li>Static array holds registered subscriber callbacks</li>
	<li>Number of registered subscribers fixed at compile time</li>
	<li>Fixed block memory allocator in C</li>
	<li>Compact implementation</li>
</ul>

<h3>Asynchronous Multicast Callbacks with Inter-Thread Messaging</h3>

<ul>
	<li>Implemented in C++</li>
	<li>Callback function is a free or static member only</li>
	<li>One callback argument supported</li>
	<li>Callback argument must be a pointer type</li>
	<li>Callback argument data copied with copy constructor</li>
	<li>Type-safety provided by templates</li>
	<li>Minimal use of templates</li>
	<li>Dynamic list of registered subscriber callbacks</li>
	<li>Number of registered subscribers expands at runtime</li>
	<li>Fixed block memory allocator in C++</li>
	<li>Compact implementation</li>
</ul>

<h3>Asynchronous Multicast Delegates in C++</h3>

<ul>
	<li>Implemented in C++</li>
	<li>C++ delegate paradigm</li>
	<li>Any callback function type (member, static, free)</li>
	<li>Multiple callback arguments supported (up to 5)</li>
	<li>Callback argument any type (value, reference, pointer, pointer to pointer)</li>
	<li>Callback argument data copied with copy constructor</li>
	<li>Type-safety provided by templates</li>
	<li>Heavy use of templates</li>
	<li>Dynamic list of registered subscriber callbacks</li>
	<li>Number of registered subscribers expands at runtime</li>
	<li>Fixed block memory allocator in C++</li>
	<li>Larger implementation</li>
</ul>

<h3>Asynchronous Multicast Delegates in Modern C++</h3>

<ul>
	<li>Implemented in C++ (i.e., C++17)</li>
	<li>C++ delegate paradigm</li>
	<li>Function signature delegate arguments</li>
	<li>Any callback function type (member, static, free)</li>
	<li>Multiple callback arguments supported (N arguments supported)</li>
	<li>Callback argument any type (value, reference, pointer, pointer to pointer)</li>
	<li>Callback argument data copied with copy constructor</li>
	<li>Type-safety provided by templates</li>
	<li>Heavy use of templates</li>
	<li>Variadic templates</li>
	<li>Template metaprogramming</li>
	<li>Dynamic list of registered subscriber callbacks</li>
	<li>Number of registered subscribers expands at runtime</li>
	<li>Compact implementation (due to variadic templates)</li>
</ul>

<h2>Conclusion</h2>

<p>I&rsquo;ve done quite a bit of multithreaded application development over the years. Invoking a function on a destination thread with data has always been a hand-crafted, time consuming process. This library generalizes those constructs and encapsulates them into a user-friendly delegate library.</p>

<p>The article proposes a C++ multicast delegate implementation supporting synchronous and asynchronous function invocation. Non-blocking asynchronous delegates offer fire-and-forget invocation whereas the blocking versions allow waiting for a return value and outgoing reference arguments from the target thread. Multicast delegate containers expand the delegate&rsquo;s usefulness by allowing multiple clients to register for callback notification. Multithreaded application development is simplified by letting the library handle the low-level threading details of invoking functions and moving data across thread boundaries. The inter-thread code is neatly hidden away within the library and users only interact with an easy to use delegate API.</p>

<h2><a name="References">References</a></h2>

<ul>
	<li><a href="https://www.codeproject.com/Articles/5277036/Asynchronous-Multicast-Delegates-in-Modern-Cpluspl"><strong>Asynchronous Multicast Delegates in Modern C++</strong></a>&nbsp;- by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/5262271/Remote-Procedure-Calls-using-Cplusplus-Delegates"><strong>Remote Procedure Calls using C++ Delegates</strong></a>&nbsp;- by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1272894/Asynchronous-Multicast-Callbacks-in-C"><strong>Asynchronous Multicast Callbacks in C</strong></a> - by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1092727/Asynchronous-Multicast-Callbacks-with-Inter-Thread"><strong>Asynchronous Multicast Callbacks with Inter-Thread Messaging</strong></a> - by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1191232/Type-Safe-Multicast-Callbacks-in-C"><strong>Type-Safe Multicast Callbacks in C</strong></a> - by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1165243/Cplusplus-State-Machine-with-Asynchronous-Multicas"><strong>C++ State Machine with Asynchronous Multicast Delegates</strong></a> - by David Lafreniere</li>
	<li><a href="http://www.codeproject.com/Articles/1084801/Replace-malloc-free-with-a-Fast-Fixed-Block-Memory"><strong>Replace malloc/free with a Fast Fixed Block Memory Allocator</strong></a> - by David Lafreniere</li>
	<li><a href="http://www.codeproject.com/Articles/1095196/Win32-Thread-Wrapper-with-Synchronized-Start"><strong>Win32 Thread Wrapper with Synchronized Start</strong></a> - by David Lafreniere</li>
	<li><strong><a href="https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible">Member Function Pointers and the Fastest Possible C++ Delegates</a></strong> - by Doug Clugston</li>
</ul>

