# OpenBMP Collector Container Image
Collector is the container for collecting BMP messages from BMP senders, e.g. routers.

## Container Includes
* Alpine Linux base image
* The latest collector (listening port is TCP 5000)
* The required libraries and runtime dependencies
* Basic command-line utilities (bash, iproute2, yq)
* An entrypoint script for the collector runtime
* A generator script for the configuration file
* A (generated) configuration file

## Building the container
See [Dockerfile](Dockerfile) notes. 

## Container Usage

### 1) Install docker
Follow the [Docker Instructions](https://docs.docker.com/installation/) to install docker.  

### 2) Pull the image

> :bulb: Running the collector for the first time will automatically pull the image.

### 3) Configuration

#### Environment Variables
Table below list the environment variables that can be used with ``docker run -e <name=value>``

All keys are preceded with **OPENBMP_** including the Kafka specific keys.

##### Global Configuration

Configuration is regenerated at container startup based on this procedure:
1. A base configuration is initialized from sane defaults in [01_generate_config](scripts/configure/01_generate_config)
2. If the **OPENBMP_CONFIG** environment variable is set, it is parsed and merged with the base configuration.
3. Remaining environment variables have the highest precedence and override all previous settings.
4. The final configuration is written to a file `/config/openbmpd.conf` and used by the collector.

> :bulb: Mounting a persistent volume to `/config` is recommended.

Please review the example configuration file at [openbmpd.conf](../Server/openbmpd.conf)

```yaml
environment:
  OPENBMP_CONFIG: |
    base:
      # The following are the default values
      # and can be overridden by environment variables
      admin_id: "collector"
      listen_port: 5000
      listen_mode: "both"
      buffers:
        router: 40
      heartbeat:
        interval: 5
      startup:
        max_concurrent_routers: 0
        initial_router_time: 60
        calculate_baseline: false
        pat_enabled: true
    ...
```

##### Base Environment Variables

| NAME | Value | Details |
| :--- | ----- | :------ |
| OPENBMP_ADMIN_ID | Name or IP | Identifying name of the collector instance. Default is 'collector' |
| OPENBMP_LISTEN_PORT | Port | Defines the port for the collector to listen on. Default is 5000. |
| OPENBMP_LISTEN_MODE | v4, v6, or both | Defines the listen mode for the collector.  Default is both. |
| OPENBMP_BUFFERS_ROUTER | Size in MB | Defines the buffer size allocated per router. Default is 40MB.<br />A size of 8MB is sufficient for a few peers.<br />Use 64MB for route reflectors and large transit peering routers.|
| OPENBMP_HEARTBEAT_INTERVAL | Minutes | Defines the interval for sending collector heartbeats to Kafka. Default is 5 minutes.<br />Consumers monitor/track this to detect if the collector and associated routers/peers are up or not. |
| OPENBMP_STARTUP_MAX_CONCURRENT_ROUTERS | Number | Defines the maximum allowed router connection for RIB dump at startup, or 0 for no limit.<br />Default is 0. |
| OPENBMP_STARTUP_INITIAL_ROUTER_TIME | Seconds | Defines the time in seconds before allowing another concurrent router. Default is 60 seconds. |
| OPENBMP_STARTUP_CALCULATE_BASELINE | Boolean | Determines if route baseline time in seconds should be calculated.<br />Default is false. |
| OPENBMP_STARTUP_PAT_ENABLED | Boolean | If false, use legacy MD5 hashes (source address, collector hash).<br />Default is true. |

##### Debugging Environment Variables

The following environment variables are used to enable debugging.  By default, all debugging is disabled.

| NAME | Value | Details |
| :--- | ----- | :------ |
| OPENBMP_DEBUG_GENERAL | Boolean | Enable general debugging. Default is false. |
| OPENBMP_DEBUG_BMP | Boolean | Enable BMP debugging. Default is false. |
| OPENBMP_DEBUG_BGP | Boolean | Enable BGP debugging. Default is false. |
| OPENBMP_DEBUG_KAFKA | Boolean | Enable Kafka debugging. Default is false. |

##### Kafka Specific Environment Variables

Kafka specific environment variables follow their Kafka naming convention.

For example, to set the Kafka environment variable **security.protocol**
use the environment variable named:  
**OPENBMP_KAFKA_SECURITY_PROTOCOL**.

| NAME | Value | Details |
| :---- | ----- | :------- |
| OPENBMP_KAFKA_BROKERS | Comma-separated string | List of Kafka brokers (e.g. `kafka1:9092,kafka2:9092`) - will be parsed into a YAML array |
| OPENBMP_KAFKA_SECURITY_PROTOCOL | String | Kafka security protocol (e.g. `SSL`) |
| OPENBMP_KAFKA_SSL_ENABLED | Boolean | Enable SSL (e.g. `true`)<br />Shortcut for setting `OPENBMP_KAFKA_SECURITY_PROTOCOL` to `SSL` |
| OPENBMP_KAFKA_SSL_CA_LOCATION | Path | CA certificate file path |
| OPENBMP_KAFKA_SSL_CERTIFICATE_LOCATION | Path | Client certificate path |
| OPENBMP_KAFKA_SSL_KEY_LOCATION | Path | Client private key path |

##### Router and Peer Group Mapping

These environment variables define parameters for assigning routers and peers to named groups.
The default is no group assignments.

| NAME | Value | Details |
| :---- | ----- | :------- |
| OPENBMP_ROUTER_GROUP_MAPPING | Multiline String | Array of router groups. |
| OPENBMP_PEER_GROUP_MAPPING | Multiline String | Array of peer groups. |

Example format for router group mapping:
```yaml
environment:
  OPENBMP_ROUTER_GROUP_MAPPING: |
    router_group:
      - name: "router_group1"
        regexp_hostname:
          - .*\.iad\..*
        prefix_range:
          - 10.100.100.0/24
          - 10.100.104.0/24
          - "2001:0:0:100::/64"
      ...
```

Example format for peer group mapping:
```yaml
environment:
  OPENBMP_PEER_GROUP_MAPPING: |
    peer_group:
      - name: "lab"
        regexp_hostname:
          - .*\.lab\..*
        prefix_range:
          - 10.100.100.0/24
          - 10.100.104.0/24
        asn:
          - 100
          - 65000
          - 65001
      ...
```

### 4) Hostnames in Container

To enable communication to services like Postgres or Kafka **hosted externally**, you can map their hostnames to IP
addresses inside the container. This can be used to make them appear local to the container.

Modifying the `/etc/hosts` file directly in the container is not possible because it is mounted **read-only** by Docker.
However, custom host entries can still be added through one of the methods below.

> :warning: When TLS/SSL is enabled, the hostname used to connect to the service must match the certificate used by the service.
> Make sure to add subject alternative names (SAN) to the certificate for all hostnames used to connect to the service.

Docker internal names should still resolve if containers are using the same bridge network.

#### a) Docker Run Command

Passing the following option to the **docker run** command will add an entry to the container's `/etc/hosts` file.
```
--add-host <HOSTNAME:IP>
```

> :bulb: Setting custom host entries this way is ephemeral and will not persist after the container is stopped, but it is useful for temporary testing.


#### b) Docker Compose
If you are using docker-compose, you can add the following to your docker-compose.yml file:

```yaml
services:
  consumer:
    image: openbmp/collector:2.2.3
    container_name: obmp-collector
    extra_hosts:
      - "kafka1:<IP>"
      - "kafka2:<IP>"
```
> :bulb: This is a better approach to maintaining custom host entries throughout the container's lifecycle.
> They will persist through container restarts until entries are changed in the `docker-compose.yml` file.


#### Docker Run Example

> :warning:
> You must define **KAFKA_BROKERS** with 'hostnames'.  If all containers are running on the same node, this
> hostname can be internal docker names, such as 'kafka1' or 'myhost'. If Kafka is running on a different server,
> than the consumers and producers, then the KAFKA_BROKERS should be a valid hostname that can be resolved using DNS.
> This can be internal DNS, or by using the docker 'extra_hosts' option.

    docker run -d --name=obmp-collector -e KAFKA_BROKERS=kafka1:9092 \
         --sysctl net.ipv4.tcp_keepalive_intvl=30 \
         --sysctl net.ipv4.tcp_keepalive_probes=5 \
         --sysctl net.ipv4.tcp_keepalive_time=180 \
         -v /var/openbmp/config:/config \
         -p 5000:5000 \
         openbmp/collector

#### Docker Compose Example

See the [docker-compose.yml](example/docker-compose.yml) for a realistic example.

### Monitoring/Troubleshooting

#### docker logs
To get the console logs, you can use
```bash
docker logs obmp-collector
```
This is useful if the container exits due to
invalid start or for another reason.

     



