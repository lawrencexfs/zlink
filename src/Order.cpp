#include "head.hpp"
#include "Order.hpp"

std::string Redis::VerifyCheckCode::_redisResponse("{ \
    \"sender\":\"redisService\", \
    \"receive\":\"web\",\
    \"level\":\"user\",\
    \"success\":\"\",\
    \"correct\":\"\",\
    \"result\":\"verifyCheckCode\"\
}");

std::string Redis::SetCheckCode::_redisResponse("{ \
    \"sender\":\"redisService\", \
    \"receive\":\"web\",\
    \"level\":\"user\",\
    \"success\":\"\",\
    \"result\":\"SetCheckCode\"\
}");

std::string SQL::Register::_sqlResponse("{ \
    \"sender\":\"dataBaseService\", \
    \"receive\":\"web\",\
    \"level\":\"user\",\
    \"email\":\"\",\
    \"result\":\"register\",\
    \"success\":0\
}");

std::string SQL::Login::_sqlResponse("{ \
    \"sender\":\"dataBaseService\", \
    \"receive\":\"web\",\
    \"level\":\"user\",\
    \"result\":\"login\",\
    \"success\":0,\
    \"correct\":0\
}");

std::string SQL::ChangerPassword::_sqlResponse("{ \
    \"sender\":\"dataBaseService\", \
    \"receive\":\"web\",\
    \"level\":\"user\",\
    \"email\":\"\",\
    \"result\":\"ChangerPassword\",\
    \"success\":0\
}");


Redis::VerifyCheckCode::VerifyCheckCode()
{
}

nlohmann::json Redis::VerifyCheckCode::constructResponse(const nlohmann::json &order, redisClient &memoryData)
{
    auto Iter = order.find("CheckCode");
    auto Iter2 = order.find("CheckCodeKeyName");
    nlohmann::json result;
    result = nlohmann::json::parse(_redisResponse);

    if (Iter == order.end() || Iter2 == order.end())
    {
        LOG(ERROR) << "not found CheckCode;order is " << order.dump();
        //....please construct err response Json

        result["success"] = 0;
        result["correct"] = 0;
        return result;
    }

    std::string checkcode = Iter.value().get<std::string>();
    std::string CheckCodeKeyName = Iter2.value().get<std::string>();
    redisReplyWrap redisReply;

    std::string str = "Get ";
    str += CheckCodeKeyName;
    memoryData.exeCommand(redisReply, str);
    if (redisReply.reply == nullptr)
    {
        //....please construct err response Json
        result["success"] = 0;
        result["correct"] = 0;
        return result;
    }

    std::string redisContainCheckCode;
    switch (redisReply.reply->type)
    {
    case REDIS_REPLY_STRING:
        redisContainCheckCode.append(redisReply.reply->str);
        break;
    default:
        LOG(ERROR) << "redis order exec err! order is " << str << CUtil::Print_trace();
        result["success"] = 0;
        result["correct"] = 0;
        return result;
    }

    result["success"] = 1;
    if (checkcode == redisContainCheckCode)
    {
        result["correct"] = 1;
    }
    else
    {
        result["correct"] = 0;
    }

    return result;
}

Redis::SetCheckCode::SetCheckCode()
{
}

nlohmann::json Redis::SetCheckCode::constructResponse(const nlohmann::json &order, redisClient &memoryData)
{
    auto Iter = order.find("CheckCode");
    auto Iter2 = order.find("CheckCodeKeyName");
    nlohmann::json result;
    result = nlohmann::json::parse(_redisResponse);
    result["success"] = 0;
    if (Iter == order.end() || Iter2 == order.end())
    {
        LOG(ERROR) << "not found CheckCode;order is " << order.dump();
        //....please construct err response Json
        return result;
    }

    std::string checkcode = Iter.value().get<std::string>();
    std::string CheckCodeKeyName = Iter2.value().get<std::string>();
    redisReplyWrap redisReply;
    std::string cmd = std::format("SET {} {}", CheckCodeKeyName, checkcode);
    memoryData.exeCommand(redisReply, cmd);
    if (redisReply.reply && strcmp(redisReply.reply->str, "OK") == 0)
    {
        //....please construct err response Json
        result["success"] = 1;
    }
    return result;
}

SQL::SQLOperation::SQLOperation()
{
}

SQL::Register::Register()
{
}

bool SQL::SQLOperation::isOrderRight(const nlohmann::json &order, std::string orType)
{
    return order.find("order").value().get<std::string>() == orType;
}

bool SQL::SQLOperation::sqlExec(std::string cmd, pgsqlClient &memoryData, pqxx::result &reply)
{
    bool ret = false;
   try{
        reply = memoryData.execCommandOneSql(cmd);
        ret = true;
    }
    catch(const pqxx::sql_error &e)
    {
        LOG(ERROR) << "SQL error: " << e.what() << std::endl;
        LOG(ERROR) << "Query was: " << e.query() << std::endl;
    } 
    catch (const std::exception &e) {
        LOG(ERROR) << "Error: " << e.what() << std::endl;
    }
    return ret;
}

nlohmann::json SQL::Register::constructResponse(const nlohmann::json &order, pgsqlClient &memoryData)
{
    nlohmann::json result;
    auto Iter0 = order.find("username");
    auto Iter1 = order.find("password");
    auto Iter2 = order.find("email");
    auto Iter3 = order.find("space_total");
    auto Iter4 = order.find("used_space");
    std::string username = Iter0.value().get<std::string>();
    std::string password = Iter1.value().get<std::string>();
    std::string email = Iter2.value().get<std::string>();
    std::string space_total = Iter3.value().get<std::string>();
    std::string used_space = Iter4.value().get<std::string>();
    result = nlohmann::json::parse(_sqlResponse);
    result["email"] = email;
    result["success"] = 0;
    if(!isOrderRight(order, "register"))
    {
        LOG(ERROR) << "order error;cur order is " << order.dump();
        return result;
    }
    if (Iter0 == order.end() || Iter1 == order.end() || Iter2 == order.end() || Iter3 == order.end() || Iter4 == order.end())
    {
        LOG(ERROR) << "The required fields are blank, please check!Cur order is " << order.dump();
        return result;
    }
    std::string cmd = std::format("INSERT INTO users (username, password, email) VALUES ('{}', '{}', '{}') RETURNING id;", username, password, email);
    pqxx::result reply;
    if(sqlExec(cmd, memoryData, reply))
    {
        int user_id = reply.size() > 0 ? reply[0][0].as<int>() : 0;
        cmd = std::format("INSERT INTO usersResource (user_id, space_total, used_space) VALUES ({}, '{}', '{}');", user_id, space_total, used_space);
        result["success"] = sqlExec(cmd, memoryData, reply);
    }
    return result;
}

SQL::Login::Login()
{
}

nlohmann::json SQL::Login::constructResponse(const nlohmann::json &order, pgsqlClient &memoryData)
{
    nlohmann::json result;
    auto Iter0 = order.find("email");
    result = nlohmann::json::parse(_sqlResponse);
    result["success"] = 0;
    result["correct"] = 0;
    if(!isOrderRight(order, "login"))
    {
        LOG(ERROR) << "order error;cur order is " << order.dump();
        return result;
    }
    if (Iter0 == order.end())
    {
        LOG(ERROR) << "The required fields are blank, please check!Cur order is " << order.dump();
        return result;
    }
    std::string email = Iter0.value().get<std::string>();
    std::string cmd = std::format("select * from users where email = '{}';", email);
    pqxx::result reply;
    if(sqlExec(cmd, memoryData, reply))
    {
        result["success"] = 1;
        std::string searchPwd = reply.size() > 0 ? reply[0][2].as<std::string>() : "";
        if(searchPwd == order.find("password").value().get<std::string>())
        {
            result["correct"] = 1;
        }
    }
    LOG(ERROR) << result.dump();
    return result;
}

SQL::ChangerPassword::ChangerPassword()
{
}

nlohmann::json SQL::ChangerPassword::constructResponse(const nlohmann::json &order, pgsqlClient &memoryData)
{
    nlohmann::json result;
    auto Iter0 = order.find("email");
    auto Iter1 = order.find("password");
    std::string email = Iter0.value().get<std::string>();
    std::string password = Iter1.value().get<std::string>();
    result = nlohmann::json::parse(_sqlResponse);
    result["email"] = email;
    result["success"] = 0;
    if(!isOrderRight(order, "ChangerPassword"))
    {
        LOG(ERROR) << "order error;cur order is " << order.dump();
        return result;
    }
    if (Iter0 == order.end())
    {
        LOG(ERROR) << "The required fields are blank, please check!Cur order is " << order.dump();
        return result;
    }
    std::string cmd = std::format("update users set password = '{}' where email = '{}' RETURNING *;", password, email);
    pqxx::result reply;
    if(sqlExec(cmd, memoryData, reply))
    {
        std::string searchPwd = reply.size() > 0 ? reply[0][2].as<std::string>() : "";
        if(searchPwd == password)
        {
            result["success"] = 1;
        }
    }
    return result;
}