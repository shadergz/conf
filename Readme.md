# conf

It's a simple configure language based on TOML syntax for rapid C based programs

## Example

A simple example can be found inside the main program

~~~c
int main()
{
    conf_t conf;

    conf_err_e err = conf_open("user.conf", &conf);

    if (err != CONF_E_OK)
        return conf_print_err(&conf);

    char pi_value[20], user_name[20];

    conf_parser("Values", "PI", pi_value, sizeof(pi_value), &conf);
    err = conf_parser("User", "username", user_name, sizeof(user_name), &conf);

    if (err != CONF_E_OK)
        return conf_print_err(&conf);

    printf("PI value: %s\n", pi_value);
    printf("Username: %s\n", user_name);

    conf_close(&conf);
}
~~~

## How to compile

~~~bash
conf> make
~~~

## How to run

~~~bash
conf> make run
~~~
