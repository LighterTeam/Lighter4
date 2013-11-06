function UserStruct (info){
    this.UUID = info.UUID;
    this.Name = info.Name;
    this.VName = info.VName;
    this.PASSWORD = info.PASSWORD;
    this.RMB = info.RMB;
    this.VRMB = info.VRMB;
    this.ICON = info.ICON;
    this.UserID = info.UserID;
    this.Gold = info.Gold;

    this.RoomFlowID = 0;
    this.CurRoomID = 0;                     // 如果是null说明当前玩家不在房间中.如果不是NULL则说明在房间中
    this.Agree = 0;                         // 玩家是否同意开始游戏
    this.FriendsApp = [];                   // 别的玩家请求的信息Info
    this.FriendsList = {};                  // 好友列表
    this.JD=0;                                //经度
    this.WD=0;
    this.iRmbType=0;//纬度
    this.iVRMB=0;
    this.OK_NO="NO"; //托管记录
    this.JJRMB="";

}

function UserStructToString (info) {
    var str =" UUID=" + info.UUID +
        " Name=" + info.Name +
        " VName=" + info.VName +
        " PASSWORD=" + info.PASSWORD +
        " RMB=" + info.RMB +
        " VRMB=" + info.VRMB +
        " ICON=" + info.ICON +
        " UserID=" + info.UserID +
        " RoomFlowID=" + info.RoomFlowID +
        " Agree=" + info.Agree;
        if(info.CurRoom != null){
            str += " CurRoomID=" + info.CurRoom.RoomID;
        }
    return str;
}

module.exports = {
    UserStruct: UserStruct,
    UserStructToString: UserStructToString
};