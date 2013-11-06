/**
 * Created with JetBrains WebStorm.
 * User: Enel
 * Date: 13-4-8
 * Time: 下午1:03
 * To change this template use File | Settings | File Templates.
 */

function UserStruct (iUUID, sName, sPassWord){
    this.UUID = iUUID;
    this.Name = sName;
    this.PassWord = sPassWord;
}

module.exports = {
    UserStruct: UserStruct
};
