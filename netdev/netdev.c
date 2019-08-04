#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>

static int ndd_init(struct net_device *dev);
static void ndd_uninit(struct net_device *dev);
static int ndd_open(struct net_device *dev);
static int ndd_stop(struct net_device *dev);
static netdev_tx_t ndd_start_xmit(struct sk_buff *skb, struct net_device *dev);

struct ndd_priv {
    struct sk_buff *skb;
};

/*
 * The network devices
 */
struct net_device *ndd_devs[2];

struct net_device_ops ndd_ops = {
    .ndo_init       = ndd_init,
    .ndo_uninit     = ndd_uninit,
    .ndo_open       = ndd_open,
    .ndo_stop       = ndd_stop,
    .ndo_start_xmit = ndd_start_xmit,
};

static int ndd_init(struct net_device *dev) {
    printk(KERN_ALERT "ndd_init [%s]\n", dev->name);
    return 0;
}

static void ndd_uninit(struct net_device *dev) {
    printk(KERN_ALERT "ndd_uninit [%s]\n", dev->name);
}

static int ndd_open(struct net_device *dev) {
    printk(KERN_ALERT "ndd_open [%s]\n", dev->name);
    netif_start_queue(dev);
    return 0;
}

static int ndd_stop(struct net_device *dev) {
    printk(KERN_ALERT "ndd_stop [%s]\n", dev->name);
    netif_stop_queue(dev);
    return 0;
}

static netdev_tx_t ndd_start_xmit(struct sk_buff *skb, struct net_device *dev) {
    struct ndd_priv *priv = netdev_priv(dev);
    int len;
    char *data;

    struct iphdr *ih;
    u32 *saddr, *daddr;

    data = skb->data;
    len = skb->len;

    priv->skb = skb;

    /* transmit a packet */
    ih = (struct iphdr *)(data + sizeof(struct ethhdr));
    saddr = &ih->saddr;
    daddr = &ih->daddr;

    return NETDEV_TX_OK;
}


static void ndd_setup(struct net_device *dev) {
    char hw_addr[] = "NDD000";
    struct ndd_priv *priv;

    ether_setup(dev);

    /* Assign the hardware address */
    memcpy(dev->dev_addr, hw_addr, ETH_ALEN);
    if (dev == ndd_devs[1]) {
        dev->dev_addr[ETH_ALEN-1]++; /* NDD001 */
    }

    /* the management hooks for network devices */
    dev->netdev_ops = &ndd_ops;

    dev->flags |= IFF_NOARP;
    // dev->features |= NETIF_F_NO_CSUM;

    priv = netdev_priv(dev);
    memset(priv, 0, sizeof(struct ndd_priv));
}


static void ndd_cleanup(void) {
    int i;

    for (i = 0; i < 2; i++) {
        if (ndd_devs[i]) {
            unregister_netdev(ndd_devs[i]);
            free_netdev(ndd_devs[i]);
        }
    }
}

static int ndd_module_init(void) {
    int i;

    ndd_devs[0] = alloc_netdev(sizeof(struct ndd_priv), "ndd%d", NET_NAME_UNKNOWN, ndd_setup);
    ndd_devs[1] = alloc_netdev(sizeof(struct ndd_priv), "ndd%d", NET_NAME_UNKNOWN, ndd_setup);
    if (NULL == ndd_devs[0] || NULL == ndd_devs[1]) {
        printk(KERN_ERR "alloc_netdev failed\n");
        goto cleanup;
    }

    for (i = 0; i < 2; i++) {
        if (register_netdev(ndd_devs[i])) {
            printk(KERN_ERR "register_netdev [%s] failed\n", ndd_devs[i]->name);
            goto cleanup;
        }
    }

    return 0;

cleanup:
    ndd_cleanup();
    return -EINVAL;
}

static void ndd_module_exit(void) {
}

module_init(ndd_module_init);
module_exit(ndd_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("joe");
MODULE_DESCRIPTION("network device driver test");
