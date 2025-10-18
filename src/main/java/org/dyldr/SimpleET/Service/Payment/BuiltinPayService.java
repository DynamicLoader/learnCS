package org.dyldr.SimpleET.Service.Payment;

import jakarta.transaction.Transactional;
import org.dyldr.SimpleET.DB.BuiltinPayAccountTable;
import org.dyldr.SimpleET.DB.BuiltinPayRecordTable;
import org.dyldr.SimpleET.Entity.BuiltinPayAccountEntity;
import org.dyldr.SimpleET.Entity.BuiltinPayRecordEntity;
import org.dyldr.SimpleET.Entity.UserEntity;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Date;
import java.util.List;

@Service
public class BuiltinPayService {

    public record BuiltinPayRet<T>(int code, T data) {
    }

    @Autowired
    private BuiltinPayAccountTable builtinPayAccountTable;

    @Autowired
    private BuiltinPayRecordTable builtinPayRecordTable;


    public BuiltinPayAccountEntity getAccount(String uid) {
        return builtinPayAccountTable.findByUser_Id(uid).orElse(null);
    }

    @Transactional
    public BuiltinPayRet<BuiltinPayAccountEntity> createAccount(String uid) {
        var acc = builtinPayAccountTable.findByUser_Id(uid).orElse(null);
        if (acc != null) { // Account already exists
            return new BuiltinPayRet<>(-1, null);
        }
        var account = new BuiltinPayAccountEntity();
        var user = new UserEntity();
        user.setId(uid);
        account.setUser(user);
        account.setBalance(0.0);
        account.setStatus(BuiltinPayAccountEntity.Status.ACTIVE);
        builtinPayAccountTable.save(account);
        return new BuiltinPayRet<>(0, account);
    }

    @Transactional
    public BuiltinPayRet<BuiltinPayRecordEntity> modify(String uid, String description, Double total) {
        var account = builtinPayAccountTable.findByUser_Id(uid).orElse(null);
        if (account == null) { // Cannot find account
            return new BuiltinPayRet<>(-1, null);
        }
        if (total < 0 && account.getBalance() < -total) { // Insufficient balance
            return new BuiltinPayRet<>(-2, null);
        }
        account.setBalance(account.getBalance() + total);
        var record = new BuiltinPayRecordEntity();
        record.setAccount(account);
        record.setAmount(total);
        record.setDescription(description);
        record.setCreateTime(new Date());
        builtinPayRecordTable.save(record);
        return new BuiltinPayRet<>(0, record);
    }


    public BuiltinPayRet<BuiltinPayRecordEntity> getRecord(String rid) {
        return builtinPayRecordTable.findById(rid).map(record -> new BuiltinPayRet<>(0, record)).orElse(new BuiltinPayRet<>(-1, null));
    }

//    public BuiltinPayRet<List<BuiltinPayRecordEntity>> getAllRecord(String uid){
//
//    }

}
