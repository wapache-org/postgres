
```
# if enable ssl support
sudo apt-get install libssl-dev

# if modify default compile settings
sudo apt-get install autoconf automake m4

```

set envirenment:

```
export PGHOST=localhost
export PGPORT=5432

export PGHOME=/opt/pgsql-12
export PGDATA=$PGHOME/data
export PGLOGS=$PGHOME/logs

export LD_LIBRARY_PATH=$PGHOME/lib
export PATH=$PGHOME/bin:$PATH

sudo mkdir $PGHOME
sudo chown postgres:postgres $PGHOME

mkdir $PGDATA
mkdir $PGLOGS
```



```
./configure --with-openssl
make
make install
```

init database and start

```
pg_ctl init
pg_ctl -l $PGLOGS/pg_ctl.log start

psql
postgres=# select version();
```

