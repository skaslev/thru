================================================
vsock/fifo/tcp/udp guest to host throughput test
================================================

Setup
-----

First, create a FIFO pair in ``/var/lib/test`` using the ``setup_fifo.sh`` script::

    you@host:~/src/vsock # ./setup_fifo.sh

Make sure that the ``vsock`` kernel module is loaded::

    you@host:~/src/vsock # modprobe vsock

Then start a guest specifying vsock cid and exposing the FIFO pair as virtserialport::

    you@host:~/src/vsock # qemu-system-x86_64 \
    -name guest2 --uuid 7edfdf4a-cb9e-11e8-b38a-173b58342476 \
    -m 4096 -boot d -enable-kvm -smp 3 -net nic -net user -hda $HOME/vm/guest2.img \
    -fsdev local,id=host,path=$HOME,security_model=mapped \
    -device virtio-9p-pci,fsdev=host,mount_tag=host \
    -device virtio-serial-pci,id=virtio-serial0,bus=pci.0,addr=0x6 \
    -chardev pipe,id=charchannel1,path=/var/lib/test/fifo0 \
    -device virtserialport,bus=virtio-serial0.0,nr=3,chardev=charchannel1,id=channel1,name=fifo0 \
    -device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=3

Use
---

To measure vsock throughput from guest to host start ``./vsock_listen`` on the host::

    you@host:~/src/vsock # ./vsock_listen
    local CID: 2

and then run ``./vsock`` on the guest::

    you@guest:~/src/vsock # ./vsock -n 1000000
    local CID: 3
    965.652985 MB/s

To measure throughput from host to guest start ``./vsock_listen`` on the guest::

    you@guest:~/src/vsock # ./vsock_listen
    local CID: 3

and then run ``./vsock`` on the host::

    you@host:~/src/vsock # ./vsock -c 3 -n 1000000
    local CID: 2
    1183.508784 MB/s

Likewise for FIFO, TCP and UDP, e.g.::

    you@host:~/src/vsock$ ./fifo_listen

    you@guest:~/src/vsock$ ./fifo -n 1000000
    3440.153267 MB/s
