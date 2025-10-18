package org.dyldr.SimpleET.Entity;

import org.apache.catalina.User;
import org.dyldr.SimpleET.Util.Cart2JsonConverter;

import lombok.Data;
import jakarta.persistence.*;

import java.util.Map;

@Entity
@Data
public class UserEntity {

    static public enum UserRole {
        // Platform side
        ADMIN,
        DEVELOPER, // ShopKeeper and Customer role

        // To B
        SHOPKEEPER,
        SHOPKEEPER_STAFF,

        // To C
        CUSTOMER;


        public static UserRole fromString(String roleStr) {
            for (UserRole r : UserRole.values()) {
                if (r.name().equalsIgnoreCase(roleStr)) {
                    return r;
                }
            }
            throw new IllegalArgumentException("No enum constant " + UserRole.class.getCanonicalName() + "." + roleStr);
        }

        public boolean canModifyProduct() {
            return this == ADMIN || this == DEVELOPER || this == SHOPKEEPER;
        }

        public boolean canModifyProductSuper() {
            return this == ADMIN;
        }
    }

    static public enum UserStatus {
        ACTIVE,
        BANNED,
        DELETED,
        REGISTERING,

        CHANGING_PASSWORD,
        CHANGING_EMAIL,
    }

    @Id
    @GeneratedValue(strategy=GenerationType.UUID)
    private String id;

    @Column(nullable = false,unique = true)
    private String name;

    @Column(nullable = false)
    private String password;

    @Column(nullable = false)
    private String salt;

    @Column(nullable = false,unique = true)
    private String email;

    @Column(nullable = false)
    @Enumerated(EnumType.STRING)
    private UserRole role;


    @Column()
    @Convert(converter = Cart2JsonConverter.class)
    private Map<String,Integer> cart;

    @Column(nullable = false)
    @Enumerated(EnumType.STRING)
    private UserStatus status;

}
