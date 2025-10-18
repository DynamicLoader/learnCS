package org.dyldr.SimpleET.Util;

import javax.mail.*;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;
import java.io.UnsupportedEncodingException;
import java.util.Properties;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

@Component
public class EmailUtil {
    @Value("${mail.smtp.host:example}")
    private String SMTP_HOST_NAME;

    @Value("${mail.smtp.auth.user:example}")
    private String SMTP_AUTH_USER;

    @Value("${mail.smtp.auth.pwd:example}")
    private String SMTP_AUTH_PWD;

    @Value("${mail.smtp.auth.alias:example}")
    private String SMTP_AUTH_ALIAS;

    public void sendEmail(String to, String subject, String body) {
        Properties properties = new Properties();
        properties.put("mail.smtp.host", SMTP_HOST_NAME);
        properties.put("mail.smtp.auth", "true");

        Authenticator auth = new SMTPAuthenticator();
        Session session = Session.getDefaultInstance(properties, auth);

        try {
            Message msg = new MimeMessage(session);
            msg.setFrom(new InternetAddress(SMTP_AUTH_USER, SMTP_AUTH_ALIAS));
            msg.setRecipient(Message.RecipientType.TO, new InternetAddress(to));
            msg.setSubject(subject);
            msg.setText(body);

            Transport.send(msg);
        } catch (MessagingException e) {
            e.printStackTrace();
        }catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    private class SMTPAuthenticator extends javax.mail.Authenticator {
        public PasswordAuthentication getPasswordAuthentication() {
            return new PasswordAuthentication(SMTP_AUTH_USER, SMTP_AUTH_PWD);
        }
    }
}