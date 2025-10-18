package org.dyldr.SimpleET.DB;

import org.dyldr.SimpleET.Entity.BuiltinPayAccountEntity;
import org.dyldr.SimpleET.Entity.UserEntity;
import org.springframework.data.repository.CrudRepository;

import java.util.Optional;

public interface BuiltinPayAccountTable extends CrudRepository<BuiltinPayAccountEntity, String> {


    Optional<BuiltinPayAccountEntity> findByUser_Id(String uid);
}
