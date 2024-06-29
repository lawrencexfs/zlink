/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: yushou-cell(email:2354739167@qq.com)
 * create: 20240620
 * FilePath: /zlink/head/redisClient.hpp
 * Description: redis client command exec
 */
#pragma once
#include "system.hpp"

class RedisClient
{
private:
    std::string _connectIP;
    int _port;
    redisContext *_client = nullptr;
    int _transactionOrderNum = 0;

public:
    RedisClient(const nlohmann::json &value);
    ~RedisClient();
    /**
     * @description: connect redis server with ip and port
     * @return {bool} connect status
     */
    bool connect();
    /**
     * @description: exec order to redis server, then redis will reply. Wrap reply
     * @param {RedisReplyWrap} &reply :redis server reply wrap
     * @param {string} &order: order of some redis operations
     */
    void exeCommand(RedisReplyWrap &reply, const std::string &order);

    /**
     * @description: used to start a Transaction
     * @return {*}
     */
    void startTransaction();

    /**
     * @description: used to add order to Transaction.Typically used in conjunction with startTransaction func.
     * @param {string} &order
     * @return {*}
     */
    void addCommandToTransaction(const std::string &order);

    /**
     * @description: Execute transaction, get response
     * @param {vector<RedisReplyWrap>} &replyArry
     * @return {*}
     */
    void exeTransaction(std::vector<RedisReplyWrap> &replyArry);
};
